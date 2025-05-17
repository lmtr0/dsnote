﻿/* Copyright (C) 2021-2025 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "speech_service.h"

#include <fmt/format.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QEventLoop>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <numeric>
#include <optional>
#include <set>

#include "april_engine.hpp"
#include "coqui_engine.hpp"
#include "ds_engine.hpp"
#include "espeak_engine.hpp"
#include "f5_engine.hpp"
#include "fasterwhisper_engine.hpp"
#include "file_source.h"
#include "gpu_tools.hpp"
#include "kokoro_engine.hpp"
#include "media_compressor.hpp"
#include "mic_source.h"
#include "mimic3_engine.hpp"
#include "module_tools.hpp"
#include "parler_engine.hpp"
#include "piper_engine.hpp"
#include "py_executor.hpp"
#include "py_tools.hpp"
#include "rhvoice_engine.hpp"
#include "sam_engine.hpp"
#include "settings.h"
#include "text_tools.hpp"
#include "vosk_engine.hpp"
#include "whisper_engine.hpp"
#include "whisperspeech_engine.hpp"

QDebug operator<<(QDebug d, const stt_engine::config_t &config) {
    std::stringstream ss;
    ss << config;

    d << QString::fromStdString(ss.str());

    return d;
}

QDebug operator<<(QDebug d, const tts_engine::config_t &config) {
    std::stringstream ss;
    ss << config;

    d << QString::fromStdString(ss.str());

    return d;
}

QDebug operator<<(QDebug d, const mnt_engine::config_t &config) {
    std::stringstream ss;
    ss << config;

    d << QString::fromStdString(ss.str());

    return d;
}

QDebug operator<<(QDebug d, const text_repair_engine::config_t &config) {
    std::stringstream ss;
    ss << config;

    d << QString::fromStdString(ss.str());

    return d;
}

QDebug operator<<(QDebug d, speech_service::state_t state_value) {
    switch (state_value) {
        case speech_service::state_t::busy:
            d << "busy";
            break;
        case speech_service::state_t::idle:
            d << "idle";
            break;
        case speech_service::state_t::listening_manual:
            d << "listening-manual";
            break;
        case speech_service::state_t::listening_auto:
            d << "listening-auto";
            break;
        case speech_service::state_t::not_configured:
            d << "not-configured";
            break;
        case speech_service::state_t::transcribing_file:
            d << "transcribing-file";
            break;
        case speech_service::state_t::listening_single_sentence:
            d << "listening-single-sentence";
            break;
        case speech_service::state_t::playing_speech:
            d << "playing-speech";
            break;
        case speech_service::state_t::writing_speech_to_file:
            d << "writing-speech-to-file";
            break;
        case speech_service::state_t::translating:
            d << "translating";
            break;
        case speech_service::state_t::repairing_text:
            d << "repairing-text";
            break;
        case speech_service::state_t::unknown:
            d << "unknown";
            break;
    }

    return d;
}

QDebug operator<<(QDebug d, speech_service::beep_role_t beep_role) {
    switch (beep_role) {
        case speech_service::beep_role_t::stt_start_listen:
            d << "stt-start-listen";
            break;
        case speech_service::beep_role_t::stt_end_listen:
            d << "stt-end-listen";
            break;
    }

    return d;
}

static bool tts_not_merge_files_options(const QVariantMap &options) {
    if (options.contains(QStringLiteral("not_merge_files"))) {
        return options.value(QStringLiteral("not_merge_files")).toBool();
    }
    return false;
}

static settings::audio_format_t tts_audio_format_from_options(
    const QVariantMap &options) {
    if (options.contains(QStringLiteral("audio_format"))) {
        auto format = options.value(QStringLiteral("audio_format")).toString();
        if (format == "wav") return settings::audio_format_t::AudioFormatWav;
        if (format == "mp3") return settings::audio_format_t::AudioFormatMp3;
        if (format == "ogg_vorbis")
            return settings::audio_format_t::AudioFormatOggVorbis;
        if (format == "ogg_opus")
            return settings::audio_format_t::AudioFormatOggOpus;
    }

    return settings::audio_format_t::AudioFormatAuto;
}

static settings::audio_quality_t tts_audio_quality_from_options(
    const QVariantMap &options) {
    if (options.contains(QStringLiteral("audio_quality"))) {
        auto quality =
            options.value(QStringLiteral("audio_quality")).toString();
        if (quality == "vbr_high")
            return settings::audio_quality_t::AudioQualityVbrHigh;
        if (quality == "vbr_medium")
            return settings::audio_quality_t::AudioQualityVbrMedium;
        if (quality == "vbr_low")
            return settings::audio_quality_t::AudioQualityVbrLow;
    }

    return settings::audio_quality_t::AudioQualityVbrMedium;
}

static QVariantList variant_list_from_list(const QStringList &list) {
    QVariantList vlist;

    vlist.reserve(list.size());
    for (const auto &item : list) {
        vlist.push_back(item);
    }

    return vlist;
}

speech_service::speech_service(QObject *parent)
    : QObject{parent}, m_dbus_service_adaptor{this} {
    qDebug() << "starting service:" << settings::launch_mode;

    connect(models_manager::instance(), &models_manager::models_changed, this,
            &speech_service::handle_models_changed);
    connect(models_manager::instance(), &models_manager::busy_changed, this,
            &speech_service::refresh_status);
    connect(models_manager::instance(), &models_manager::download_progress,
            this, [this](const QString &id, double progress) {
                emit model_download_progress(id, progress);
            });
    connect(this, &speech_service::stt_text_decoded, this,
            static_cast<void (speech_service::*)(const QString &,
                                                 const QString &, int)>(
                &speech_service::handle_stt_text_decoded),
            Qt::QueuedConnection);
    connect(this, &speech_service::sentence_timeout, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_stt_sentence_timeout),
            Qt::QueuedConnection);
    connect(this, &speech_service::stt_engine_eof, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_stt_engine_eof),
            Qt::QueuedConnection);
    connect(this, &speech_service::stt_engine_error, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_stt_engine_error),
            Qt::QueuedConnection);
    connect(this, &speech_service::stt_engine_stopped, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_stt_engine_stopped),
            Qt::QueuedConnection);
    connect(this, &speech_service::stt_engine_stopping, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_stt_engine_stopping),
            Qt::QueuedConnection);
    connect(this, &speech_service::stt_engine_state_changed, this,
            &speech_service::handle_stt_engine_state_changed,
            Qt::QueuedConnection);
    connect(this, &speech_service::tts_engine_error, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_tts_engine_error),
            Qt::QueuedConnection);
    connect(this, &speech_service::text_repair_engine_error, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_text_repair_engine_error),
            Qt::QueuedConnection);
    connect(this, &speech_service::mnt_engine_error, this,
            static_cast<void (speech_service::*)(mnt_engine::error_t, int)>(
                &speech_service::handle_mnt_engine_error),
            Qt::QueuedConnection);
    connect(this, &speech_service::mnt_engine_translate_progress_changed, this,
            static_cast<void (speech_service::*)(int)>(
                &speech_service::handle_mnt_progress_changed),
            Qt::QueuedConnection);
    connect(this, &speech_service::mnt_engine_state_changed, this,
            &speech_service::handle_mnt_engine_state_changed,
            Qt::QueuedConnection);
    connect(this, &speech_service::tts_engine_state_changed, this,
            &speech_service::handle_tts_engine_state_changed,
            Qt::QueuedConnection);
    connect(this, &speech_service::text_repair_engine_state_changed, this,
            &speech_service::handle_text_repair_engine_state_changed,
            Qt::QueuedConnection);
    connect(this, &speech_service::tts_speech_encoded, this,
            static_cast<void (speech_service::*)(tts_partial_result_t)>(
                &speech_service::handle_tts_speech_encoded),
            Qt::QueuedConnection);
    connect(this, &speech_service::ttt_text_repaired, this,
            &speech_service::handle_ttt_text_repaired, Qt::QueuedConnection);
    connect(this, &speech_service::stt_engine_shutdown, this,
            [this] { stop_stt_engine(); });
    connect(
        this, &speech_service::requet_update_task_state, this,
        [this] { update_task_state(); }, Qt::QueuedConnection);
    connect(&m_player, &QMediaPlayer::stateChanged, this,
            &speech_service::handle_player_state_changed, Qt::QueuedConnection);
    connect(
        settings::instance(), &settings::default_stt_model_changed, this,
        [this]() {
            if (settings::launch_mode == settings::launch_mode_t::service) {
                auto model = default_stt_model();
                qDebug() << "[service => dbus] signal "
                            "DefaultSttModelPropertyChanged:"
                         << model;
                emit DefaultSttModelPropertyChanged(model);
            }

            emit default_stt_model_changed();

            if (settings::launch_mode == settings::launch_mode_t::service) {
                auto lang = default_stt_lang();
                qDebug()
                    << "[service => dbus] signal DefaultSttLangPropertyChanged:"
                    << lang;
                emit DefaultSttLangPropertyChanged(lang);
            }

            emit default_stt_lang_changed();
        },
        Qt::QueuedConnection);
    connect(
        settings::instance(), &settings::default_tts_model_changed, this,
        [this]() {
            if (settings::launch_mode == settings::launch_mode_t::service) {
                auto model = default_tts_model();
                qDebug() << "[service => dbus] signal "
                            "DefaultTtsModelPropertyChanged:"
                         << model;
                emit DefaultTtsModelPropertyChanged(model);
            }

            emit default_tts_model_changed();

            if (settings::launch_mode == settings::launch_mode_t::service) {
                auto lang = default_tts_lang();
                qDebug()
                    << "[service => dbus] signal DefaultTtsLangPropertyChanged:"
                    << lang;
                emit DefaultTtsLangPropertyChanged(lang);
            }

            emit default_tts_lang_changed();
        },
        Qt::QueuedConnection);
    connect(
        settings::instance(), &settings::default_mnt_lang_changed, this,
        [this]() {
            qDebug() << "default_mnt_lang_changed:"
                     << settings::instance()->default_mnt_lang();
            if (settings::launch_mode == settings::launch_mode_t::service) {
                auto lang = default_mnt_lang();
                qDebug() << "[service => dbus] signal "
                            "DefaultMntLangPropertyChanged:"
                         << lang;
                emit DefaultMntLangPropertyChanged(lang);
            }

            emit default_mnt_lang_changed();

            settings::instance()->set_default_mnt_out_lang(
                default_mnt_out_lang());
        },
        Qt::QueuedConnection);
    connect(
        settings::instance(), &settings::default_mnt_out_lang_changed, this,
        [this]() {
            if (settings::launch_mode == settings::launch_mode_t::service) {
                auto lang = default_mnt_out_lang();
                qDebug() << "[service => dbus] signal "
                            "DefaultMntOutLangPropertyChanged:"
                         << lang;
                emit DefaultMntOutLangPropertyChanged(lang);
            }

            emit default_mnt_out_lang_changed();
        },
        Qt::QueuedConnection);

    if (settings::launch_mode == settings::launch_mode_t::service) {
        connect(
            this, &speech_service::state_changed, this,
            [this]() {
                qDebug() << "[service => dbus] signal StatePropertyChanged:"
                         << dbus_state();
                emit StatePropertyChanged(dbus_state());
            },
            Qt::QueuedConnection);
        connect(
            this, &speech_service::current_task_changed, this,
            [this]() {
                qDebug()
                    << "[service => dbus] signal CurrentTaskPropertyChanged:"
                    << current_task_id();
                emit CurrentTaskPropertyChanged(current_task_id());
            },
            Qt::QueuedConnection);
        connect(this, &speech_service::error, this, [this](error_t type) {
            qDebug() << "[service => dbus] signal ErrorOccured:"
                     << static_cast<int>(type);
            emit ErrorOccured(static_cast<int>(type));
        });
        connect(this, &speech_service::stt_file_transcribe_finished, this,
                [this](int task) {
                    qDebug()
                        << "[service => dbus] signal SttFileTranscribeFinished:"
                        << task;
                    emit SttFileTranscribeFinished(task);
                });
        connect(
            this, &speech_service::stt_intermediate_text_decoded, this,
            [this](const QString &text, const QString &lang, int task) {
                qDebug()
                    << "[service => dbus] signal SttIntermediateTextDecoded:"
                    << lang << task;
                emit SttIntermediateTextDecoded(text, lang, task);
            });
        connect(this, &speech_service::stt_text_decoded, this,
                [this](const QString &text, const QString &lang, int task) {
                    qDebug()
                        << "[service => dbus] signal SttTextDecoded:" << lang
                        << task;
                    emit SttTextDecoded(text, lang, task);
                });
        connect(this, &speech_service::tts_partial_speech_playing, this,
                [this](const QString &text, int task) {
                    qDebug()
                        << "[service => dbus] signal TtsPartialSpeechPlaying:"
                        << task;
                    emit TtsPartialSpeechPlaying(text, task);
                });
        connect(this, &speech_service::tts_speech_to_file_finished, this,
                [this](const QStringList &files, int task) {
                    qDebug()
                        << "[service => dbus] signal TtsSpeechToFileFinished:"
                        << task;
                    emit TtsSpeechToFileFinished(files, task);
                });
        connect(this, &speech_service::ttt_repair_text_finished, this,
                [this](const QString &text, int task) {
                    qDebug()
                        << "[service => dbus] signal TtsRestoreTextFinished:"
                        << task;
                    emit TtrRepairTextFinished(text, task);
                });
        connect(this, &speech_service::tts_speech_to_file_progress_changed,
                this, [this](double progress, int task) {
                    qDebug()
                        << "[service => dbus] signal TtsSpeechToFileProgress:"
                        << progress << task;
                    emit TtsSpeechToFileProgress(progress, task);
                });
        connect(this, &speech_service::mnt_translate_progress_changed, this,
                [this](double progress, int task) {
                    qDebug() << "[service => dbus] signal MntTranslateProgress:"
                             << progress << task;
                    emit MntTranslateProgress(progress, task);
                });
        connect(
            this, &speech_service::mnt_translate_finished, this,
            [this](const QString &in_text, const QString &in_lang,
                   const QString &out_text, const QString &out_lang, int task) {
                qDebug() << "[service => dbus] signal MntTranslateFinished:"
                         << task;
                emit MntTranslateFinished(in_text, in_lang, out_text, out_lang,
                                          task);
            });
        connect(
            this, &speech_service::task_state_changed, this,
            [this]() {
                qDebug() << "[service => dbus] signal TaskStatePropertyChanged:"
                         << task_state();
                emit TaskStatePropertyChanged(task_state());
            },
            Qt::QueuedConnection);
        connect(this, &speech_service::models_changed, this, [this]() {
            // stt

            auto models_list = available_stt_models();
            qDebug() << "[service => dbus] signal SttModelsPropertyChanged";
            emit SttModelsPropertyChanged(models_list);

            auto langs_list = available_stt_langs();
            qDebug() << "[service => dbus] signal SttLangsPropertyChanged";
            emit SttLangsPropertyChanged(langs_list);

            auto lang_list = available_stt_lang_list();
            qDebug() << "[service => dbus] signal SttLangListPropertyChanged";
            emit SttLangListPropertyChanged(lang_list);

            // tts

            models_list = available_tts_models();
            qDebug() << "[service => dbus] signal TtsModelsPropertyChanged";
            emit TtsModelsPropertyChanged(models_list);

            langs_list = available_tts_langs();
            qDebug() << "[service => dbus] signal TtsLangsPropertyChanged";
            emit TtsLangsPropertyChanged(langs_list);

            lang_list = available_tts_lang_list();
            qDebug() << "[service => dbus] signal TtsLangListPropertyChanged";
            emit TtsLangListPropertyChanged(lang_list);

            // stt + tts

            lang_list = available_stt_tts_lang_list();
            qDebug()
                << "[service => dbus] signal SttTtsLangListPropertyChanged";
            emit SttTtsLangListPropertyChanged(lang_list);

            // ttt

            models_list = available_ttt_models();
            qDebug() << "[service => dbus] signal TttModelsPropertyChanged";
            emit TttModelsPropertyChanged(models_list);

            langs_list = available_ttt_langs();
            qDebug() << "[service => dbus] signal TttLangsPropertyChanged";
            emit TttLangsPropertyChanged(langs_list);

            // mnt
            
            langs_list = available_mnt_langs();
            qDebug() << "[service => dbus] signal MntLangsPropertyChanged";
            emit MntLangsPropertyChanged(langs_list);

            lang_list = available_mnt_lang_list();
            qDebug() << "[service => dbus] signal MntLangListPropertyChanged";
            emit MntLangListPropertyChanged(lang_list);

            m_models_changed_handled = true;

            refresh_status();
        });
        connect(this, &speech_service::stt_transcribe_file_progress_changed,
                this, [this](double progress, int task) {
                    qDebug()
                        << "[service => dbus] signal SttFileTranscribeProgress:"
                        << progress << task;
                    emit SttFileTranscribeProgress(progress, task);
                });
        connect(this, &speech_service::tts_play_speech_finished, this,
                [this](int task) {
                    qDebug()
                        << "[service => dbus] signal TtsPlaySpeechFinished:"
                        << task;
                    emit TtsPlaySpeechFinished(task);
                });
        connect(
            this, &speech_service::features_availability_updated, this,
            [this]() {
                qDebug()
                    << "[service => dbus] signal FeaturesAvailabilityUpdated";
                emit FeaturesAvailabilityUpdated();
            },
            Qt::QueuedConnection);

        m_keepalive_timer.setSingleShot(true);
        m_keepalive_timer.setTimerType(Qt::VeryCoarseTimer);
        m_keepalive_timer.setInterval(KEEPALIVE_TIME);
        connect(&m_keepalive_timer, &QTimer::timeout, this,
                &speech_service::handle_keepalive_timeout);

        m_keepalive_current_task_timer.setSingleShot(true);
        m_keepalive_current_task_timer.setTimerType(Qt::VeryCoarseTimer);
        m_keepalive_current_task_timer.setInterval(KEEPALIVE_TASK_TIME);
        connect(&m_keepalive_current_task_timer, &QTimer::timeout, this,
                &speech_service::handle_task_timeout);

        // DBus
        auto con = QDBusConnection::sessionBus();
        if (!con.registerService(DBUS_SERVICE_NAME)) {
            qWarning() << "dbus service registration failed";
            throw std::runtime_error("dbus service registration failed");
        }
        if (!con.registerObject(DBUS_SERVICE_PATH, this)) {
            qWarning() << "dbus object registration failed";
            throw std::runtime_error("dbus object registration failed");
        }
    } else {
        connect(this, &speech_service::models_changed, this, [this] {
            m_models_changed_handled = true;
            refresh_status();
        });
    }

    m_features_availability_timer.setSingleShot(true);
    m_features_availability_timer.setTimerType(Qt::VeryCoarseTimer);
    m_features_availability_timer.setInterval(1000);
    connect(&m_features_availability_timer, &QTimer::timeout, this, [this]() {
        qDebug() << "trying features availability update:"
                 << static_cast<bool>(
                        py_executor::instance()->libs_availability);
        if (py_executor::instance()->libs_availability) {
            features_availability();
        } else {
            m_features_availability_timer.start();
        }
    });

    if (settings::launch_mode == settings::launch_mode_t::service) {
        m_keepalive_timer.start();
    }

    setup_env();

    py_executor::instance()->start();

    setup_modules();

    remove_cached_files();

    handle_models_changed();

    features_availability();
}

speech_service::~speech_service() { qDebug() << "speech service dtor"; }

speech_service::source_t speech_service::audio_source_type() const {
    if (!m_source) return source_t::none;
    if (m_source->type() == audio_source::source_type::mic)
        return source_t::mic;
    if (m_source->type() == audio_source::source_type::file)
        return source_t::file;
    return source_t::none;
}

void speech_service::fill_available_models_map(
    const std::vector<models_manager::model_t> &models) {
    lang_to_model_map_t lang_to_model_map;

    m_available_stt_models_map.clear();
    m_available_tts_models_map.clear();
    m_available_ttt_models_map.clear();
    m_available_mnt_models_map.clear();

    auto default_stt_model = settings::instance()->default_stt_model();
    auto default_tts_model = settings::instance()->default_tts_model();
    auto default_mnt_lang = settings::instance()->default_mnt_lang();

    for (const auto &model : models) {
        auto role = models_manager::role_of_engine(model.engine);

        switch (role) {
            case models_manager::model_role_t::stt:
                m_available_stt_models_map.emplace(
                    model.id,
                    model_data_t{model.id, model.lang_id, model.trg_lang_id,
                                 model.engine, model.name, model.options});
                lang_to_model_map.stt.emplace(model.lang_id, model.id);
                if (model.id == default_stt_model)
                    lang_to_model_map.found_default_stt = true;
                break;
            case models_manager::model_role_t::ttt:
                m_available_ttt_models_map.emplace(
                    model.id,
                    model_data_t{model.id, model.lang_id, model.trg_lang_id,
                                 model.engine, model.name, model.options});
                break;
            case models_manager::model_role_t::tts:
                m_available_tts_models_map.emplace(
                    model.id,
                    model_data_t{model.id, model.lang_id, model.trg_lang_id,
                                 model.engine, model.name, model.options});
                lang_to_model_map.tts.emplace(model.lang_id, model.id);
                if (model.id == default_tts_model)
                    lang_to_model_map.found_default_tts = true;
                break;
            case models_manager::model_role_t::mnt:
                m_available_mnt_models_map.emplace(
                    model.id,
                    model_data_t{model.id, model.lang_id, model.trg_lang_id,
                                 model.engine, model.name, model.options});
                lang_to_model_map.mnt.emplace(model.lang_id, model.id);
                if (model.lang_id == default_mnt_lang)
                    lang_to_model_map.found_default_mnt = true;
                break;
        }
    }

    if (!lang_to_model_map.found_default_stt) {
        qDebug() << "default stt model not found:" << default_stt_model;
        QString lang;

        if (!default_stt_model.contains('_'))
            lang = default_stt_model.split('_').first();

        if (auto it = lang_to_model_map.stt.find(lang);
            it != lang_to_model_map.stt.end()) {
            default_stt_model = it->second;
            qDebug() << "new default stt model by lang:" << default_stt_model;
        } else if (auto it = lang_to_model_map.stt.find("en");
                   it != lang_to_model_map.stt.end()) {
            default_stt_model = it->second;
            qDebug() << "new default stt model by en:" << default_stt_model;
        } else if (!lang_to_model_map.stt.empty()) {
            default_stt_model = lang_to_model_map.stt.cbegin()->second;
            qDebug() << "new default stt model by first:" << default_stt_model;
        }

        settings::instance()->set_default_stt_model(default_stt_model);
    }

    if (!lang_to_model_map.found_default_tts) {
        qDebug() << "default tts model not found:" << default_tts_model;
        QString lang;

        if (!default_tts_model.contains('_'))
            lang = default_tts_model.split('_').first();

        if (auto it = lang_to_model_map.tts.find(lang);
            it != lang_to_model_map.tts.end()) {
            default_tts_model = it->second;
            qDebug() << "new default tts model by lang:" << default_tts_model;
        } else if (auto it = lang_to_model_map.tts.find(QStringLiteral("en"));
                   it != lang_to_model_map.tts.end()) {
            default_tts_model = it->second;
            qDebug() << "new default tts model by en:" << default_tts_model;
        } else if (!lang_to_model_map.tts.empty()) {
            default_tts_model = lang_to_model_map.tts.cbegin()->second;
            qDebug() << "new default tts model by first:" << default_tts_model;
        }

        settings::instance()->set_default_tts_model(default_tts_model);
    }

    if (!lang_to_model_map.found_default_mnt) {
        qDebug() << "default mnt lang not found:" << default_mnt_lang;

        if (lang_to_model_map.mnt.count(QStringLiteral("en")) != 0) {
            default_mnt_lang = "en";
            qDebug() << "new default mnt lang by en:" << default_mnt_lang;
        } else if (!lang_to_model_map.mnt.empty()) {
            default_mnt_lang = lang_to_model_map.mnt.cbegin()->first;
            qDebug() << "new default mnt lang by first:" << default_mnt_lang;
        }

        qDebug() << "new default mnt lang:" << default_mnt_lang;
        settings::instance()->set_default_mnt_lang(default_mnt_lang);
    }

    m_available_mnt_lang_to_model_id_map = std::move(lang_to_model_map.mnt);
}

bool speech_service::matched_engine_type(engine_t engine_type,
                                         models_manager::model_engine_t engine) {
    auto role = models_manager::role_of_engine(engine);
    if (engine_type == engine_t::stt && role == models_manager::model_role_t::stt)
        return true;
    if (engine_type == engine_t::tts && role == models_manager::model_role_t::tts)
        return true;
    if (engine_type == engine_t::mnt && role == models_manager::model_role_t::mnt)
        return true;
    return false;
}

std::optional<speech_service::model_config_t>
speech_service::choose_model_config_by_id(
    const std::vector<models_manager::model_t> &models, engine_t engine_type,
    const QString &model_or_lang_id, const QString &out_lang_id) {
    std::optional<speech_service::model_config_t> config;

    auto it =
        std::find_if(models.cbegin(), models.cend(), [&](const auto &model) {
            if (!matched_engine_type(engine_type, model.engine)) return false;
            return engine_type == engine_t::mnt
                       ? (model_or_lang_id == model.lang_id &&
                          out_lang_id == model.trg_lang_id)
                       : model_or_lang_id == model.id;
        });

    if (it != models.cend()) {
        const auto model = *it;
        config.emplace();
        switch (engine_type) {
            case engine_t::stt: {
                auto scorer_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::scorer, model.sup_files);
                auto openvino_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::openvino,
                    model.sup_files);
                config->stt = stt_model_config_t{
                    model.lang_id, model.lang_code, model.id, model.engine,
                    model.model_file,
                    /*scorer_file=*/
                    scorer_file ? scorer_file->get().file : QString{},
                    /*openvino_file=*/
                    openvino_file ? openvino_file->get().file : QString{},
                    /*ttt=*/{}};
                break;
            }
            case engine_t::tts: {
                auto vocoder_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::vocoder, model.sup_files);
                auto diacritizer_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::diacritizer,
                    model.sup_files);
                config->tts = tts_model_config_t{
                    model.lang_id, model.lang_code, model.id, model.engine,
                    model.model_file,
                    /*vocoder_file=*/
                    vocoder_file ? vocoder_file->get().file : QString{},
                    /*diacritizer_file=*/
                    diacritizer_file ? diacritizer_file->get().file : QString{},
                    model.speaker};
                break;
            }
            case engine_t::mnt:
                config->mnt = mnt_model_config_t{
                    model.lang_id,           out_lang_id, model.id,
                    model.model_file,
                    /*model_id_second=*/{},
                    /*model_file_second=*/{}};
                break;
            case engine_t::text_repair:
                break;
        }
        config->options = model.options;
    }

    if (!config && engine_type == engine_t::mnt && model_or_lang_id != "en" &&
        out_lang_id != "en") {
        // search for lang_id=>en and en=>out_lang_id

        auto it_model_to_en = std::find_if(
            models.cbegin(), models.cend(), [&](const auto &model) {
                if (models_manager::role_of_engine(model.engine) !=
                    models_manager::model_role_t::mnt)
                    return false;
                return model_or_lang_id == model.lang_id &&
                       model.trg_lang_id == "en";
            });

        if (it_model_to_en != models.cend()) {
            auto it_model_from_en = std::find_if(
                models.cbegin(), models.cend(), [&](const auto &model) {
                    if (models_manager::role_of_engine(model.engine) !=
                        models_manager::model_role_t::mnt)
                        return false;
                    return model.lang_id == "en" &&
                           model.trg_lang_id == out_lang_id;
                });

            if (it_model_from_en != models.cend()) {
                const auto model_first = *it_model_to_en;
                const auto model_second = *it_model_from_en;
                config.emplace();
                config->mnt = mnt_model_config_t{
                    model_first.lang_id, out_lang_id,
                    model_first.id,      model_first.model_file,
                    model_second.id,     model_second.model_file};
                config->options = model_first.options;
                qDebug() << "mnt both models found";
            }
        }
    }

    return config;
}

std::optional<speech_service::model_config_t>
speech_service::choose_model_config_by_lang(
    const std::vector<models_manager::model_t> &models, engine_t engine_type,
    const QString &model_or_lang_id) {
    if (engine_type != engine_t::stt && engine_type != engine_t::tts)
        throw std::runtime_error("invalid engine type");

    std::optional<speech_service::model_config_t> config;

    int best_score = -1;
    const models_manager::model_t *best_model = nullptr;

    for (const auto &model : models) {
        if (!matched_engine_type(engine_type, model.engine)) continue;
        if (model_or_lang_id == model.lang_id) {
            if (model.default_for_lang) {
                best_model = &model;
                qDebug() << "best model is default model for lang:"
                         << model.lang_id << model.id;
                break;
            }

            if (model.score > best_score) {
                best_model = &model;
                best_score = model.score;
            }
        }
    }

    if (best_model) {
        config.emplace();
        switch (engine_type) {
            case engine_t::stt: {
                auto scorer_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::scorer,
                    best_model->sup_files);
                auto openvino_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::openvino,
                    best_model->sup_files);
                config->stt = stt_model_config_t{
                    best_model->lang_id, best_model->lang_code, best_model->id,
                    best_model->engine, best_model->model_file,
                    /*scorer_file=*/
                    scorer_file ? scorer_file->get().file : QString{},
                    /*openvino_file=*/
                    openvino_file ? openvino_file->get().file : QString{},
                    /*ttt=*/{}};
                break;
            }
            case engine_t::tts: {
                auto vocoder_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::vocoder,
                    best_model->sup_files);
                auto diacritizer_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::diacritizer,
                    best_model->sup_files);
                config->tts = tts_model_config_t{
                    best_model->lang_id, best_model->lang_code, best_model->id,
                    best_model->engine, best_model->model_file,
                    /*vocoder_file=*/
                    vocoder_file ? vocoder_file->get().file : QString{},
                    /*diacritizer_file=*/
                    diacritizer_file ? diacritizer_file->get().file : QString{},
                    best_model->speaker};
                break;
            }
            case engine_t::text_repair:
            case engine_t::mnt:
                break;
        }
        config->options = best_model->options;
    }

    return config;
}

std::optional<speech_service::model_config_t>
speech_service::choose_model_config_by_first(
    const std::vector<models_manager::model_t> &models, engine_t engine_type) {
    if (engine_type != engine_t::stt && engine_type != engine_t::tts)
        throw std::runtime_error("invalid engine type");

    std::optional<speech_service::model_config_t> config;

    auto it =
        std::find_if(models.cbegin(), models.cend(), [&](const auto &model) {
            return matched_engine_type(engine_type, model.engine);
        });

    if (it != models.cend()) {
        const auto model = *it;
        config.emplace();
        switch (engine_type) {
            case engine_t::stt: {
                auto scorer_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::scorer, model.sup_files);
                auto openvino_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::openvino,
                    model.sup_files);
                config->stt = stt_model_config_t{
                    model.lang_id, model.lang_code, model.id, model.engine,
                    model.model_file,
                    /*scorer_file=*/
                    scorer_file ? scorer_file->get().file : QString{},
                    /*openvino_file=*/
                    openvino_file ? scorer_file->get().file : QString{},
                    /*ttt=*/{}};
                break;
            }
            case engine_t::tts: {
                auto vocoder_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::vocoder, model.sup_files);
                auto diacritizer_file = models_manager::sup_model_file_of_role(
                    models_manager::sup_model_role_t::diacritizer,
                    model.sup_files);
                config->tts = tts_model_config_t{
                    model.lang_id, model.lang_code, model.id, model.engine,
                    model.model_file,
                    /*vocoder_file=*/
                    vocoder_file ? vocoder_file->get().file : QString{},
                    /*diacritizer_file=*/
                    diacritizer_file ? diacritizer_file->get().file : QString{},
                    model.speaker};
                break;
            }
            case engine_t::text_repair:
            case engine_t::mnt:
                break;
        }
        config->options = model.options;
    }

    return config;
}

std::optional<speech_service::model_config_t>
speech_service::choose_model_config(engine_t engine_type,
                                    QString model_or_lang_id,
                                    QString out_lang_id) {
    auto models = models_manager::instance()->available_models();

    fill_available_models_map(models);

    if (models.empty()) {
        qDebug() << "no models available";
        return std::nullopt;
    }

    if (engine_type == engine_t::text_repair) {
        speech_service::model_config_t config;
        config.text_repair.emplace();

        if (auto it = std::find_if(
                models.cbegin(), models.cend(),
                [&](const auto &model) {
                    return model.engine ==
                           models_manager::model_engine_t::ttt_tashkeel;
                });
            it != models.cend()) {
            config.text_repair->diacritizer_ar = {it->id, it->model_file};
        } else {
            qDebug() << "can't find arabic diacritization model";
        }

        if (auto it = std::find_if(
                models.cbegin(), models.cend(),
                [&](const auto &model) {
                    return model.engine ==
                           models_manager::model_engine_t::ttt_unikud;
                });
            it != models.cend()) {
            config.text_repair->diacritizer_he = {it->id, it->model_file};
        } else {
            qDebug() << "can't find hebrew diacritization model";
        }

        if (auto it = std::find_if(
                models.cbegin(), models.cend(),
                [&](const auto &model) {
                    return model.engine ==
                           models_manager::model_engine_t::ttt_hftc;
                });
            it != models.cend()) {
            config.text_repair->punctuation = {it->id, it->model_file};
        } else {
            qDebug() << "can't find punctuation model";
        }

        return config;
    }

    if (model_or_lang_id.isEmpty()) {
        switch (engine_type) {
            case engine_t::stt:
                model_or_lang_id = settings::instance()->default_stt_model();
                break;
            case engine_t::tts:
                model_or_lang_id = settings::instance()->default_tts_model();
                break;
            case engine_t::mnt:
                model_or_lang_id = settings::instance()->default_mnt_lang();
                break;
            case engine_t::text_repair:
                break;
        }
    }

    if (out_lang_id.isEmpty()) {
        out_lang_id = settings::instance()->default_mnt_out_lang();
    }

    model_or_lang_id = model_or_lang_id.toLower();
    out_lang_id = out_lang_id.toLower();

    qDebug() << "choosing model for id:" << model_or_lang_id << out_lang_id;

    auto active_config = choose_model_config_by_id(
        models, engine_type, model_or_lang_id, out_lang_id);

    if (!active_config && engine_type == engine_t::mnt) {
        qDebug() << "no mnt model for lang available";
        return std::nullopt;
    }

    if (!active_config) {
        active_config =
            choose_model_config_by_lang(models, engine_type, model_or_lang_id);
    }

    if (!active_config) {
        active_config = choose_model_config_by_first(models, engine_type);

        if (active_config) {
            QString model_id;
            switch (engine_type) {
                case engine_t::stt:
                    model_id = active_config->stt->model_id;
                    break;
                case engine_t::tts:
                    model_id = active_config->tts->model_id;
                case engine_t::text_repair:
                case engine_t::mnt:
                    break;
            }
            qWarning() << "cannot find requested model, choosing:" << model_id;
        }
    }

    if (!active_config) {
        qWarning() << "cannot find any model";
        return active_config;
    }

    // search for ttt model for stt lang
    // only when restore punctuation is enabled
    if (engine_type == engine_t::stt && active_config->stt &&
        settings::instance()->restore_punctuation()) {
        auto it = std::find_if(
            models.cbegin(), models.cend(), [&](const auto &model) {
                return model.engine ==
                           models_manager::model_engine_t::ttt_hftc &&
                       model.lang_id == active_config->stt->lang_id;
            });

        if (it != models.cend()) {
            qDebug() << "found ttt model for stt:" << it->id;

            active_config->stt->ttt =
                ttt_model_config_t{it->id, it->model_file};
        }
    }

    return active_config;
}

void speech_service::handle_models_changed() {
    fill_available_models_map(models_manager::instance()->available_models());

    if (m_current_task &&
        (m_available_stt_models_map.find(m_current_task->model_id) ==
             m_available_stt_models_map.end() ||
         m_available_tts_models_map.find(m_current_task->model_id) ==
             m_available_tts_models_map.end() ||
         m_available_mnt_models_map.find(m_current_task->model_id) ==
             m_available_mnt_models_map.end())) {
        stop_stt();
    }

    emit models_changed();
}

QString speech_service::lang_from_model_id(const QString &model_id) {
    auto l = model_id.split('_');

    if (l.empty()) {
        qDebug() << "invalid model id:" << model_id;
        return {};
    }

    return l.first();
}

bool speech_service::get_bool_value_from_options(const QString &name,
                                                 bool default_value,
                                                 const QVariantMap &options) {
    if (options.contains(name))
        return options.value(name, default_value).toBool();
    return default_value;
}

int speech_service::get_int_value_from_options(const QString &name,
                                               int default_value,
                                               const QVariantMap &options) {
    if (options.contains(name)) {
        bool ok = false;
        auto value = options.value(name).toInt(&ok);
        if (ok) return value;
    }
    return default_value;
}

QString speech_service::get_string_value_from_options(
    const QString &name, const QString &default_value,
    const QVariantMap &options) {
    if (options.contains(name))
        return options.value(name, default_value).toString();
    return default_value;
}

template <typename Engine>
static std::optional<typename Engine::gpu_device_t> make_gpu_device(
    const QString &gpu_str, const QString &auto_device_str) {
    auto l = gpu_str.split(',');
    if (l.size() <= 2) l = auto_device_str.split(',');

    qDebug() << "gpu device str:" << l;

    if (l.size() > 2) {
        if (l.at(0).trimmed() == "OpenCL") {
            typename Engine::gpu_device_t device;
            device.api = Engine::gpu_api_t::opencl;
            device.id = 0;
            device.name = l.at(2).trimmed().toStdString();
            device.platform_name = l.at(1).trimmed().toStdString();
            return device;
        }
        if (l.at(0).trimmed() == "CUDA") {
            typename Engine::gpu_device_t device;
            device.api = Engine::gpu_api_t::cuda;
            device.id = l.at(1).trimmed().toInt();
            device.name = l.at(2).trimmed().toStdString();
            return device;
        }
        if (l.at(0).trimmed() == "ROCm") {
            typename Engine::gpu_device_t device;
            device.api = Engine::gpu_api_t::rocm;
            device.id = l.at(1).trimmed().toInt();
            device.name = l.at(2).trimmed().toStdString();
            return device;
        }
        if (l.at(0).trimmed() == "OpenVINO") {
            typename Engine::gpu_device_t device;
            device.api = Engine::gpu_api_t::openvino;
            device.id = 0;
            device.name = l.at(1).trimmed().toStdString();
            return device;
        }
        if (l.at(0).trimmed() == "Vulkan") {
            typename Engine::gpu_device_t device;
            device.api = Engine::gpu_api_t::vulkan;
            device.id = l.at(1).trimmed().toInt();
            device.name = l.at(2).trimmed().toStdString();
            return device;
        }
    } else {
        qWarning() << "invalid gpu device str:" << gpu_str << auto_device_str;
    }

    return std::nullopt;
}

static stt_engine::text_format_t stt_text_fromat_from_settings_format(
    settings::text_format_t format) {
    switch (format) {
        case settings::text_format_t::TextFormatRaw:
            return stt_engine::text_format_t::raw;
        case settings::text_format_t::TextFormatSubRip:
            return stt_engine::text_format_t::subrip;
        case settings::text_format_t::TextFormatMarkdown:
        case settings::text_format_t::TextFormatHtml:
            break;
    }

    throw std::runtime_error("invalid text format");
}

static stt_engine::sub_config_t stt_sub_config_from_options(
    const QVariantMap &options) {
    stt_engine::sub_config_t sub_config{};

    if (auto k = QStringLiteral("sub_min_segment_dur"); options.contains(k))
        sub_config.min_segment_dur = options.value(k).toUInt();
    if (auto k = QStringLiteral("sub_min_line_length"); options.contains(k))
        sub_config.min_line_length = options.value(k).toUInt();
    if (auto k = QStringLiteral("sub_max_line_length"); options.contains(k))
        sub_config.max_line_length = options.value(k).toUInt();

    return sub_config;
}

static std::vector<int> whispercpp_vulkan_devices() {
    std::vector<int> devs;

    auto dev_strs = settings::instance()->whispercpp_gpu_devices();
    if (dev_strs.size() > 1) {
        for (auto it = std::next(dev_strs.cbegin()); it != dev_strs.cend();
             ++it) {
            auto l = it->split(',');
            if (l.size() <= 2 || l.at(0).trimmed() != "Vulkan") continue;
            devs.push_back(l.at(1).trimmed().toInt());
        }
    }

    return devs;
}

QString speech_service::restart_stt_engine(speech_mode_t speech_mode,
                                           const QString &model_id,
                                           const QString &out_lang_id,
                                           const QVariantMap &options) {
    auto model_config = choose_model_config(engine_t::stt, model_id);
    if (model_config && model_config->stt) {
        stt_engine::config_t config;

        config.model_files.model_file =
            model_config->stt->model_file.toStdString();
        config.model_files.scorer_file =
            model_config->stt->scorer_file.toStdString();
        config.model_files.openvino_model_file =
            model_config->stt->openvino_file.toStdString();
        if (model_config->stt->ttt)
            config.model_files.ttt_model_file =
                model_config->stt->ttt->model_file.toStdString();
        config.lang = model_config->stt->lang_id.toStdString();
        config.lang_code = model_config->stt->lang_code.toStdString();
        config.speech_mode =
            static_cast<stt_engine::speech_mode_t>(speech_mode);
        config.translate = !out_lang_id.isEmpty() && out_lang_id == "en" &&
                           config.lang != "en";
        config.options = model_config->options.toStdString();
        config.text_format = stt_text_fromat_from_settings_format(
            static_cast<settings::text_format_t>(get_int_value_from_options(
                "text_format",
                static_cast<int>(settings::text_format_t::TextFormatRaw),
                options)));
        config.sub_config = stt_sub_config_from_options(options);
        config.cache_dir = settings::instance()->cache_dir().toStdString();
        config.insert_stats =
            get_bool_value_from_options("insert_stats", false, options);
        config.initial_prompt =
            get_string_value_from_options("initial_prompt", {}, options)
                .toStdString();

        // clang-format off
#define ENGINE_OPTS(name_) \
        switch(settings::instance()->name_##_profile()) {  \
        case settings::engine_profile_t::EngineProfilePerformance: \
            config.beam_search = 2; \
            config.cpu_threads = 4; \
            config.audio_ctx_conf = stt_engine::audio_ctx_conf_t::dynamic; \
            config.audio_ctx_size = 1500; \
            break; \
        case settings::engine_profile_t::EngineProfileQuality: \
            config.beam_search = 5; \
            config.cpu_threads = 4; \
            config.audio_ctx_conf = stt_engine::audio_ctx_conf_t::no_change; \
            config.audio_ctx_size = 1500; \
            break; \
        case settings::engine_profile_t::EngineProfileCustom: \
            config.beam_search = settings::instance()->name_##_beam_search(); \
            config.cpu_threads = settings::instance()->name_##_cpu_threads(); \
            config.audio_ctx_conf = []{ \
                switch(settings::instance()->name_##_audioctx_size()) { \
                case settings::option_t::OptionAuto: return stt_engine::audio_ctx_conf_t::dynamic; \
                case settings::option_t::OptionDefault: return stt_engine::audio_ctx_conf_t::no_change; \
                case settings::option_t::OptionCustom: return stt_engine::audio_ctx_conf_t::custom; \
                } \
                return stt_engine::audio_ctx_conf_t::dynamic; \
            }(); \
            config.audio_ctx_size = settings::instance()->name_##_audioctx_size_value(); \
            break; \
        } \
        if (settings::instance()->name_##_use_gpu() && settings::instance()->has_##name_##_gpu_device()) { \
            if (auto device = make_gpu_device<stt_engine>(settings::instance()->name_##_gpu_device(), settings::instance()->name_##_auto_gpu_device())) { \
                config.gpu_device = std::move(*device); \
                if (settings::instance()->name_##_profile() == settings::engine_profile_t::EngineProfileCustom) { \
                    config.gpu_device.flash_attn = \
                        (config.gpu_device.api == stt_engine::gpu_api_t::cuda || config.gpu_device.api == stt_engine::gpu_api_t::rocm) && \
                        settings::instance()->name_##_gpu_flash_attn() && \
                        config.gpu_device.name.find("ZLUDA") == std::string::npos; \
                } else { \
                    config.gpu_device.flash_attn = false; \
                } \
                config.use_gpu = true; \
            } \
        }
        
        if (model_config->stt->engine == models_manager::model_engine_t::stt_whisper) {
            ENGINE_OPTS(whispercpp)
            if (config.gpu_device.api == stt_engine::gpu_api_t::vulkan) {
                config.available_devices = whispercpp_vulkan_devices();
            }
            if (!settings::instance()->whispercpp_autolang_with_sup()) {
                // disable auto-lang with sup model
                config.model_files.scorer_file.clear();
            }
        } else if (model_config->stt->engine == models_manager::model_engine_t::stt_fasterwhisper) {
            ENGINE_OPTS(fasterwhisper)
        }
#undef ENGINE_OPTS
        // clang-format on

        bool new_engine_required = [&] {
            if (!m_stt_engine) return true;

            const auto &type = typeid(*m_stt_engine);
            if (model_config->stt->engine ==
                    models_manager::model_engine_t::stt_ds &&
                type != typeid(ds_engine))
                return true;
            if (model_config->stt->engine ==
                    models_manager::model_engine_t::stt_vosk &&
                type != typeid(vosk_engine))
                return true;
            if (model_config->stt->engine ==
                    models_manager::model_engine_t::stt_whisper &&
                type != typeid(whisper_engine))
                return true;
            if (model_config->stt->engine ==
                    models_manager::model_engine_t::stt_fasterwhisper &&
                type != typeid(fasterwhisper_engine))
                return true;
            if (model_config->stt->engine ==
                    models_manager::model_engine_t::stt_april &&
                type != typeid(april_engine))
                return true;

            if (m_stt_engine->model_files() != config.model_files) return true;
            if (m_stt_engine->model_files() != config.model_files) return true;
            if (m_stt_engine->lang() != config.lang) return true;
            if (m_stt_engine->translate() != config.translate) return true;
            if (config.use_gpu != m_stt_engine->use_gpu() ||
                config.gpu_device != m_stt_engine->gpu_device())
                return true;
            if (m_stt_engine->audio_ctx_conf() != config.audio_ctx_conf)
                return true;
            if (m_stt_engine->audio_ctx_size() != config.audio_ctx_size)
                return true;
            if (m_stt_engine->cpu_threads() != config.cpu_threads) return true;
            if (m_stt_engine->beam_search() != config.beam_search) return true;

            return false;
        }();

        qDebug() << "restart stt engine config:" << config;

        if (new_engine_required) {
            qDebug() << "new stt engine required";

            if (m_stt_engine) {
                m_stt_engine.reset();
                qDebug() << "stt engine destroyed successfully";
            }

            stt_engine::callbacks_t call_backs{
                /*text_decoded=*/[this](const std::string &text,
                                        const std::string &lang) {
                    handle_stt_text_decoded(text, lang);
                },
                /*intermediate_text_decoded=*/
                [this](const std::string &text, const std::string &lang) {
                    handle_stt_intermediate_text_decoded(text, lang);
                },
                /*speech_detection_status_changed=*/
                [this](stt_engine::speech_detection_status_t status) {
                    if (m_current_task)
                        emit stt_engine_state_changed(status,
                                                      m_current_task->id);
                },
                /*sentence_timeout=*/
                [this]() {
                    if (m_current_task && m_current_task->speech_mode ==
                                              speech_mode_t::single_sentence) {
                        emit sentence_timeout(m_current_task->id);
                    }
                },
                /*eof=*/
                [this]() {
                    if (m_current_task) emit stt_engine_eof(m_current_task->id);
                },
                /*error=*/
                [this]() {
                    if (m_current_task)
                        emit stt_engine_error(m_current_task->id);
                },
                /*stopping=*/
                [this]() {
                    if (m_current_task)
                        emit stt_engine_stopping(m_current_task->id);
                },
                /*stopped=*/
                [this]() {
                    if (m_current_task)
                        emit stt_engine_stopped(m_current_task->id);
                }};

            try {
                switch (model_config->stt->engine) {
                    case models_manager::model_engine_t::stt_ds:
                        m_stt_engine = std::make_unique<ds_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::stt_vosk:
                        m_stt_engine = std::make_unique<vosk_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::stt_whisper:
                        m_stt_engine = std::make_unique<whisper_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::stt_fasterwhisper:
                        m_stt_engine = std::make_unique<fasterwhisper_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::stt_april:
                        m_stt_engine = std::make_unique<april_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::ttt_hftc:
                    case models_manager::model_engine_t::ttt_tashkeel:
                    case models_manager::model_engine_t::ttt_unikud:
                    case models_manager::model_engine_t::tts_coqui:
                    case models_manager::model_engine_t::tts_piper:
                    case models_manager::model_engine_t::tts_espeak:
                    case models_manager::model_engine_t::tts_rhvoice:
                    case models_manager::model_engine_t::tts_mimic3:
                    case models_manager::model_engine_t::tts_whisperspeech:
                    case models_manager::model_engine_t::tts_sam:
                    case models_manager::model_engine_t::tts_parler:
                    case models_manager::model_engine_t::tts_f5:
                    case models_manager::model_engine_t::tts_kokoro:
                    case models_manager::model_engine_t::mnt_bergamot:
                        throw std::runtime_error{
                            "invalid model engine, expected stt"};
                }
            } catch (const std::runtime_error &err) {
                qWarning() << "failed to create stt engine:" << err.what();
                emit error(error_t::stt_engine);
                return {};
            }

            m_stt_engine->start();
        } else {
            qDebug() << "new stt engine not required, only restart";
            m_stt_engine->stop();
            m_stt_engine->start();
            m_stt_engine->set_speech_mode(
                static_cast<stt_engine::speech_mode_t>(speech_mode));
            m_stt_engine->set_text_format(config.text_format);
            m_stt_engine->set_sub_config(config.sub_config);
            m_stt_engine->set_insert_stats(config.insert_stats);
            m_stt_engine->set_initial_prompt(std::move(config.initial_prompt));
        }

        return model_config->stt->model_id;
    }

    qWarning() << "failed to restart stt engine, no valid model";
    return {};
}

static tts_engine::audio_format_t format_from_cache_format(
    settings::cache_audio_format_t format) {
    switch (format) {
        case settings::cache_audio_format_t::CacheAudioFormatWav:
            return tts_engine::audio_format_t::wav;
        case settings::cache_audio_format_t::CacheAudioFormatMp3:
            return tts_engine::audio_format_t::mp3;
        case settings::cache_audio_format_t::CacheAudioFormatOggVorbis:
            return tts_engine::audio_format_t::ogg_vorbis;
        case settings::cache_audio_format_t::CacheAudioFormatOggOpus:
            return tts_engine::audio_format_t::ogg_opus;
        case settings::cache_audio_format_t::CacheAudioFormatFlac:
            return tts_engine::audio_format_t::flac;
    }

    throw std::runtime_error{"invalid format"};
}

static tts_engine::subtitles_sync_mode_t tts_subtitles_sync_mode_from_settings(
    settings::tts_subtitles_sync_mode_t mode) {
    switch (mode) {
        case settings::tts_subtitles_sync_mode_t::TtsSubtitleSyncOff:
            return tts_engine::subtitles_sync_mode_t::off;
        case settings::tts_subtitles_sync_mode_t::TtsSubtitleSyncOnDontFit:
            return tts_engine::subtitles_sync_mode_t::on_dont_fit;
        case settings::tts_subtitles_sync_mode_t::TtsSubtitleSyncOnAlwaysFit:
            return tts_engine::subtitles_sync_mode_t::on_always_fit;
        case settings::tts_subtitles_sync_mode_t::
            TtsSubtitleSyncOnFitOnlyIfLonger:
            return tts_engine::subtitles_sync_mode_t::on_fit_only_if_longer;
    }

    throw std::runtime_error("invalid subtitles sync mode");
}

static tts_engine::text_format_t tts_text_fromat_from_settings_format(
    settings::text_format_t format) {
    switch (format) {
        case settings::text_format_t::TextFormatSubRip:
            return tts_engine::text_format_t::subrip;
        case settings::text_format_t::TextFormatRaw:
        case settings::text_format_t::TextFormatMarkdown:
        case settings::text_format_t::TextFormatHtml:
            return tts_engine::text_format_t::raw;
    }

    throw std::runtime_error("invalid text format");
}

static tts_engine::tag_mode_t tts_tag_mode_from_settings_tag_mode(
    settings::tts_tag_mode_t mode) {
    switch (mode) {
        case settings::tts_tag_mode_t::TtsTagModeDisable:
            return tts_engine::tag_mode_t::disable;
        case settings::tts_tag_mode_t::TtsTagModeIgnore:
            return tts_engine::tag_mode_t::ignore;
        case settings::tts_tag_mode_t::TtsTagModeSupport:
            return tts_engine::tag_mode_t::support;
    }

    throw std::runtime_error("invalid tag mode");
}

QString speech_service::restart_tts_engine(const QString &model_id,
                                           const QVariantMap &options) {
    auto model_config = choose_model_config(engine_t::tts, model_id);
    if (model_config && model_config->tts) {
        tts_engine::config_t config;

        config.model_files.model_path =
            model_config->tts->model_file.toStdString();
        config.model_files.vocoder_path =
            model_config->tts->vocoder_file.toStdString();
        config.model_files.hub_path = QFileInfo{model_config->tts->model_file}
                                          .dir()
                                          .absolutePath()
                                          .toStdString();
        if (settings::instance()->diacritizer_enabled())
            config.model_files.diacritizer_path =
                model_config->tts->diacritizer_file.toStdString();
        config.lang = model_config->tts->lang_id.toStdString();
        config.cache_dir = settings::instance()->cache_dir().toStdString();
        config.speaker_id = model_config->tts->speaker.toStdString();
        config.options = model_config->options.toStdString();
        config.text_format = tts_text_fromat_from_settings_format(
            static_cast<settings::text_format_t>(get_int_value_from_options(
                "text_format",
                static_cast<int>(settings::text_format_t::TextFormatRaw),
                options)));
        config.sync_subs = tts_subtitles_sync_mode_from_settings(
            static_cast<
                settings::tts_subtitles_sync_mode_t>(get_int_value_from_options(
                "sync_subs",
                static_cast<int>(
                    settings::tts_subtitles_sync_mode_t::TtsSubtitleSyncOff),
                options)));
        config.audio_format = format_from_cache_format(
            settings::instance()->cache_audio_format());
        config.ref_voice_file =
            get_string_value_from_options("ref_voice_file", {}, options)
                .toStdString();
        config.lang_code = model_config->tts->lang_code.toStdString();
        config.split_into_sentences =
            get_bool_value_from_options("split_into_sentences", true, options);
        config.use_engine_speed_control = get_bool_value_from_options(
            "use_engine_speed_control", true, options);
        config.normalize_audio =
            get_bool_value_from_options("normalize_audio", true, options);
        config.speech_speed = std::clamp(
            get_int_value_from_options("speech_speed", 10, options), 1, 20);
        config.tag_mode = tts_tag_mode_from_settings_tag_mode(
            static_cast<settings::tts_tag_mode_t>(get_int_value_from_options(
                "tag_mode",
                static_cast<int>(
                    settings::settings::tts_tag_mode_t::TtsTagModeSupport),
                options)));
        config.ref_prompt =
            get_string_value_from_options("ref_prompt", {}, options)
                .toStdString();

        // clang-format off
#define X(name) \
        if (settings::instance()->name##_use_gpu() && settings::instance()->has_##name##_gpu_device()) { \
            if (auto device = make_gpu_device<tts_engine>(settings::instance()->name##_gpu_device(), settings::instance()->name##_auto_gpu_device())) { \
                config.gpu_device = std::move(*device); \
                config.use_gpu = true; \
            } \
        }
        
        if (model_config->tts->engine == models_manager::model_engine_t::tts_coqui) {
            X(coqui)
        } else if (model_config->tts->engine == models_manager::model_engine_t::tts_whisperspeech) {
            X(whisperspeech)
        } else if (model_config->tts->engine == models_manager::model_engine_t::tts_parler) {
            X(parler)
        } else if (model_config->tts->engine == models_manager::model_engine_t::tts_f5) {
            X(f5)
        } else if (model_config->tts->engine == models_manager::model_engine_t::tts_kokoro) {
            X(kokoro)
        }
#undef X
        // clang-format on

        if (model_config->tts->model_id.contains("fairseq")) {
            auto l = model_config->tts->model_id.split('_');
            if (!l.isEmpty()) {
                config.lang_code = l.last().toStdString();
                config.share_dir = module_tools::path_to_share_dir_for_path(
                                       "uroman/bin/uroman.pl")
                                       .toStdString();
            }
        } else {
            config.share_dir =
                module_tools::path_to_share_dir_for_path("/libnumbertext")
                    .toStdString();
        }

        QFile nb_file{QStringLiteral(":/nonbreaking_prefixes/%1.txt")
                          .arg(model_config->tts->lang_id.split('-').first())};
        if (nb_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            config.nb_data = nb_file.readAll().toStdString();
        } else {  // fallback to en
            QFile nb_file_en{QStringLiteral(":/nonbreaking_prefixes/en.txt")};
            if (nb_file_en.open(QIODevice::ReadOnly | QIODevice::Text)) {
                config.nb_data = nb_file_en.readAll().toStdString();
            }
        }

        bool new_engine_required = [&] {
            if (!m_tts_engine) return true;

            const auto &type = typeid(*m_tts_engine);
            if (model_config->tts->engine ==
                models_manager::model_engine_t::tts_coqui &&
                type != typeid(coqui_engine))
                return true;
            if (model_config->tts->engine ==
                models_manager::model_engine_t::tts_piper &&
                type != typeid(piper_engine))
                return true;
            if (model_config->tts->engine ==
                models_manager::model_engine_t::tts_rhvoice &&
                type != typeid(rhvoice_engine))
                return true;
            if (model_config->tts->engine ==
                models_manager::model_engine_t::tts_mimic3 &&
                type != typeid(mimic3_engine))
                return true;
            if (model_config->tts->engine ==
                    models_manager::model_engine_t::tts_whisperspeech &&
                type != typeid(whisperspeech_engine))
                return true;
            if (model_config->tts->engine ==
                    models_manager::model_engine_t::tts_espeak &&
                type != typeid(espeak_engine))
                return true;
            if (model_config->tts->engine ==
                    models_manager::model_engine_t::tts_sam &&
                type != typeid(sam_engine))
                return true;
            if (model_config->tts->engine ==
                    models_manager::model_engine_t::tts_parler &&
                type != typeid(parler_engine))
                return true;
            if (model_config->tts->engine ==
                    models_manager::model_engine_t::tts_f5 &&
                type != typeid(f5_engine))
                return true;
            if (model_config->tts->engine ==
                    models_manager::model_engine_t::tts_kokoro &&
                type != typeid(kokoro_engine))
                return true;

            if (m_tts_engine->model_files() != config.model_files) return true;

            bool engine_restart_when_speaker_changed =
                model_config->tts->engine ==
                    models_manager::model_engine_t::tts_piper ||
                model_config->tts->engine ==
                    models_manager::model_engine_t::tts_rhvoice ||
                model_config->tts->engine ==
                    models_manager::model_engine_t::tts_espeak ||
                model_config->tts->engine ==
                    models_manager::model_engine_t::tts_kokoro;
            if (engine_restart_when_speaker_changed &&
                m_tts_engine->speaker() != config.speaker_id)
                return true;

            bool engine_restart_when_lang_changed =
                model_config->tts->engine ==
                models_manager::model_engine_t::tts_espeak;
            if (engine_restart_when_lang_changed &&
                m_tts_engine->lang() != config.lang)
                return true;

            // bool engine_restart_when_ref_prompt_changed =
            //     model_config->tts->engine ==
            //     models_manager::model_engine_t::tts_parler;
            // if (engine_restart_when_ref_prompt_changed &&
            //     m_tts_engine->ref_prompt() != config.ref_prompt)
            //     return true;

            if (config.use_gpu != m_tts_engine->use_gpu() ||
                config.gpu_device != m_tts_engine->gpu_device())
                return true;

            return false;
        }();

        qDebug() << "restart tts engine config:" << config;

        if (new_engine_required) {
            qDebug() << "new tts engine required";

            if (m_tts_engine) {
                m_tts_engine.reset();
                qDebug() << "tts engine destroyed successfully";
            }

            tts_engine::callbacks_t call_backs{
                /*speech_encoded=*/[this](
                                       const std::string &text,
                                       const std::string &audio_file_path,
                                       tts_engine::audio_format_t audio_format,
                                       double progress, bool last) {
                    handle_tts_speech_encoded(text, audio_file_path,
                                              audio_format, progress, last);
                },
                /*text_restored=*/
                [this](const std::string &text) {
                    if (m_current_task) {
                        emit ttt_text_repaired(QString::fromStdString(text),
                                               m_current_task->id);
                    }
                },
                /*state_changed=*/
                [this](tts_engine::state_t state) {
                    if (m_current_task) {
                        emit tts_engine_state_changed(state,
                                                      m_current_task->id);
                    }
                },
                /*error=*/
                [this]() {
                    if (m_current_task)
                        emit tts_engine_error(m_current_task->id);
                }};

            try {
                switch (model_config->tts->engine) {
                    case models_manager::model_engine_t::tts_coqui:
                        m_tts_engine = std::make_unique<coqui_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_piper:
                        config.data_dir =
                            module_tools::unpacked_dir("espeakdata")
                                .toStdString();
                        m_tts_engine = std::make_unique<piper_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_espeak:
                        config.data_dir =
                            module_tools::unpacked_dir("espeakdata")
                                .toStdString();
                        m_tts_engine = std::make_unique<espeak_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_rhvoice:
                        config.data_dir =
                            module_tools::unpacked_dir("rhvoicedata")
                                .toStdString();
                        config.config_dir =
                            module_tools::unpacked_dir("rhvoiceconfig")
                                .toStdString();
                        m_tts_engine = std::make_unique<rhvoice_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_mimic3:
                        config.data_dir =
                            module_tools::unpacked_dir("mimic3").toStdString();
                        m_tts_engine = std::make_unique<mimic3_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_whisperspeech:
                        m_tts_engine = std::make_unique<whisperspeech_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_sam:
                        m_tts_engine = std::make_unique<sam_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_parler:
                        m_tts_engine = std::make_unique<parler_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_f5:
                        m_tts_engine = std::make_unique<f5_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::tts_kokoro:
                        m_tts_engine = std::make_unique<kokoro_engine>(
                            std::move(config), std::move(call_backs));
                        break;
                    case models_manager::model_engine_t::ttt_hftc:
                    case models_manager::model_engine_t::ttt_tashkeel:
                    case models_manager::model_engine_t::ttt_unikud:
                    case models_manager::model_engine_t::stt_ds:
                    case models_manager::model_engine_t::stt_vosk:
                    case models_manager::model_engine_t::stt_whisper:
                    case models_manager::model_engine_t::stt_fasterwhisper:
                    case models_manager::model_engine_t::stt_april:
                    case models_manager::model_engine_t::mnt_bergamot:
                        throw std::runtime_error{
                            "invalid model engine, expected tts"};
                }
            } catch (const std::runtime_error &err) {
                qWarning() << "failed to create tts engine:" << err.what();
                emit error(error_t::tts_engine);
                return {};
            }
        } else {
            qDebug() << "new tts engine not required";
            m_tts_engine->set_speech_speed(config.speech_speed);
            m_tts_engine->set_ref_voice_file(std::move(config.ref_voice_file));
            m_tts_engine->set_ref_prompt(std::move(config.ref_prompt));
            m_tts_engine->set_text_format(config.text_format);
            m_tts_engine->set_sync_subs(config.sync_subs);
            m_tts_engine->set_split_into_sentences(config.split_into_sentences);
            m_tts_engine->set_use_engine_speed_control(
                config.use_engine_speed_control);
            m_tts_engine->set_speaker(config.speaker_id);
            m_tts_engine->set_lang(config.lang);
            m_tts_engine->set_lang_code(config.lang_code);
            m_tts_engine->set_tag_mode(config.tag_mode);
            m_tts_engine->set_normalize_audio(config.normalize_audio);
            m_tts_engine->restart();
        }

        m_tts_engine->start();

        return model_config->tts->model_id;
    }

    qWarning() << "failed to restart tts engine, no valid model";
    return {};
}

static mnt_engine::text_format_t mnt_text_fromat_from_settings_format(
    settings::text_format_t format) {
    switch (format) {
        case settings::text_format_t::TextFormatRaw:
            return mnt_engine::text_format_t::raw;
        case settings::text_format_t::TextFormatHtml:
            return mnt_engine::text_format_t::html;
        case settings::text_format_t::TextFormatMarkdown:
            return mnt_engine::text_format_t::markdown;
        case settings::text_format_t::TextFormatSubRip:
            return mnt_engine::text_format_t::subrip;
    }

    throw std::runtime_error("invalid text format");
}

QString speech_service::restart_mnt_engine(const QString &model_or_lang_id,
                                           const QString &out_lang_id,
                                           const QVariantMap &options) {
    auto model_config =
        choose_model_config(engine_t::mnt, model_or_lang_id, out_lang_id);
    if (model_config && model_config->mnt) {
        mnt_engine::config_t config;

        config.model_files.model_path_first =
            model_config->mnt->model_file_first.toStdString();
        config.model_files.model_path_second =
            model_config->mnt->model_file_second.toStdString();
        config.lang = model_config->mnt->lang_id.toStdString();
        config.out_lang = model_config->mnt->out_lang_id.toStdString();
        config.options = model_config->options.toStdString();
        config.clean_text =
            get_bool_value_from_options("clean_text", false, options);
        config.text_format = mnt_text_fromat_from_settings_format(
            static_cast<settings::text_format_t>(get_int_value_from_options(
                "text_format",
                static_cast<int>(settings::text_format_t::TextFormatRaw),
                options)));

        QFile nb_file{QStringLiteral(":/nonbreaking_prefixes/%1.txt")
                          .arg(model_config->mnt->lang_id.split('-').first())};
        if (nb_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            config.nb_data = nb_file.readAll().toStdString();
        } else {  // fallback to en
            QFile nb_file_en{QStringLiteral(":/nonbreaking_prefixes/en.txt")};
            if (nb_file_en.open(QIODevice::ReadOnly | QIODevice::Text)) {
                config.nb_data = nb_file_en.readAll().toStdString();
            }
        }

        bool new_engine_required = [&] {
            if (!m_mnt_engine) return true;
            if (m_mnt_engine->model_files() != config.model_files) return true;
            if (m_mnt_engine->lang() != config.lang) return true;

            return false;
        }();

        qDebug() << "restart mnt engine config:" << config;

        if (new_engine_required) {
            qDebug() << "new mnt engine required";

            if (m_mnt_engine) {
                m_tts_engine.reset();
                qDebug() << "mnt engine destroyed successfully";
            }

            mnt_engine::callbacks_t call_backs{
                /*text_translated=*/
                [this](const std::string &in_text, const std::string &in_lang,
                       std::string &&out_text, const std::string &out_lang) {
                    handle_mnt_translate_finished(
                        in_text, in_lang, std::move(out_text), out_lang);
                },
                /*state_changed=*/
                [this](mnt_engine::state_t state) {
                    if (m_current_task) {
                        emit mnt_engine_state_changed(state,
                                                      m_current_task->id);
                    }
                },
                /*progress_changed=*/
                [this]() {
                    if (m_current_task)
                        emit mnt_engine_translate_progress_changed(
                            m_current_task->id);
                },
                /*error=*/
                [this](mnt_engine::error_t error_type) {
                    handle_mnt_engine_error(error_type);
                }};

            try {
                m_mnt_engine = std::make_unique<mnt_engine>(
                    std::move(config), std::move(call_backs));
            } catch (const std::runtime_error &err) {
                qWarning() << "failed to create mnt engine:" << err.what();
                emit error(error_t::mnt_engine);
                return {};
            }
        } else {
            qDebug() << "new mnt engine not required";

            m_mnt_engine->set_clean_text(config.clean_text);
            m_mnt_engine->set_text_format(config.text_format);
        }

        m_mnt_engine->start();

        return model_config->mnt->model_id_first;
    }

    qWarning() << "failed to restart mnt engine, no valid model";
    return {};
}

static text_repair_engine::text_format_t
text_repair_text_fromat_from_settings_format(settings::text_format_t format) {
    switch (format) {
        case settings::text_format_t::TextFormatSubRip:
            return text_repair_engine::text_format_t::subrip;
        case settings::text_format_t::TextFormatRaw:
        case settings::text_format_t::TextFormatMarkdown:
        case settings::text_format_t::TextFormatHtml:
            return text_repair_engine::text_format_t::raw;
    }

    throw std::runtime_error("invalid text format");
}

bool speech_service::restart_text_repair_engine(const QVariantMap &options) {
    auto model_config = choose_model_config(engine_t::text_repair);
    if (model_config && model_config->text_repair) {
        text_repair_engine::config_t config;

        if (model_config->text_repair->diacritizer_ar)
            config.model_files.diacritizer_path_ar =
                model_config->text_repair->diacritizer_ar->model_file
                    .toStdString();
        if (model_config->text_repair->diacritizer_he)
            config.model_files.diacritizer_path_he =
                model_config->text_repair->diacritizer_he->model_file
                    .toStdString();
        if (model_config->text_repair->punctuation)
            config.model_files.punctuator_path =
                model_config->text_repair->punctuation->model_file
                    .toStdString();
        config.options = model_config->options.toStdString();
        config.text_format = text_repair_text_fromat_from_settings_format(
            static_cast<settings::text_format_t>(get_int_value_from_options(
                "text_format",
                static_cast<int>(settings::text_format_t::TextFormatRaw),
                options)));
        config.share_dir =
            module_tools::path_to_share_dir_for_path("/libnumbertext")
                .toStdString();

        bool new_engine_required = [&] {
            if (!m_text_repair_engine) return true;

            if (config.use_gpu != m_text_repair_engine->use_gpu() ||
                config.gpu_device != m_text_repair_engine->gpu_device())
                return true;

            return false;
        }();

        qDebug() << "restart text-repair engine config:" << config;

        if (new_engine_required) {
            qDebug() << "new text-repair engine required";

            if (m_text_repair_engine) {
                m_text_repair_engine.reset();
                qDebug() << "text-repair engine destroyed successfully";
            }

            text_repair_engine::callbacks_t call_backs{
                /*text_repaired=*/[this](const std::string &text) {
                    if (m_current_task)
                        emit ttt_text_repaired(QString::fromStdString(text),
                                               m_current_task->id);
                },
                /*state_changed=*/
                [this](text_repair_engine::state_t state) {
                    if (m_current_task)
                        emit text_repair_engine_state_changed(
                            state, m_current_task->id);
                },
                /*error=*/
                [this]() {
                    if (m_current_task)
                        emit text_repair_engine_error(m_current_task->id);
                }};

            try {
                m_text_repair_engine = std::make_unique<text_repair_engine>(
                    std::move(config), std::move(call_backs));
            } catch (const std::runtime_error &err) {
                qWarning() << "failed to create text-repair engine:"
                           << err.what();
                emit error(error_t::text_repair_engine);
                return {};
            }
        } else {
            qDebug() << "new text-repair engine not required";
            m_text_repair_engine->set_text_format(config.text_format);
        }

        m_text_repair_engine->start();

        return true;
    }

    qWarning() << "failed to restart text-repair engine, no valid model";
    return false;
}

void speech_service::handle_stt_intermediate_text_decoded(
    const std::string &text, const std::string &lang) {
    if (m_current_task) {
        m_last_intermediate_text_task = m_current_task->id;
        emit stt_intermediate_text_decoded(QString::fromStdString(text),
                                           QString::fromStdString(lang),
                                           m_current_task->id);
    } else {
        qWarning() << "current task does not exist";
    }
}

void speech_service::handle_stt_text_decoded(
    [[maybe_unused]] const QString &text, [[maybe_unused]] const QString &lang,
    int task_id) {
    if (m_current_task && m_current_task->id == task_id) {
        if (m_current_task->speech_mode == speech_mode_t::single_sentence) {
            stt_stop_listen(m_current_task->id);
        }
    }
}

void speech_service::handle_stt_sentence_timeout(int task_id) {
    stt_stop_listen(task_id);
}

void speech_service::handle_stt_engine_eof(int task_id) {
    qDebug() << "stt engine eof:" << task_id;

    if (!m_current_task || m_current_task->id != task_id ||
        m_current_task->engine != engine_t::stt) {
        qDebug() << "ignoring stt eof";
        return;
    }

    if (audio_source_type() == source_t::file && m_stt_engine &&
        !m_stt_engine->stop_requested())
        emit stt_file_transcribe_finished(task_id);
    cancel(task_id);
}

void speech_service::handle_stt_engine_error(int task_id) {
    qDebug() << "stt engine error:" << task_id;

    emit error(error_t::stt_engine);

    if (current_task_id() == task_id) {
        cancel(task_id);
        if (m_stt_engine) {
            m_stt_engine.reset();
            qDebug() << "stt engine destroyed successfully";
        }
    }
}

void speech_service::handle_stt_engine_stopped(int task_id) {
    qDebug() << "stt engine stopped:" << task_id;

    if (current_task_id() == task_id) stop_stt_engine();
}

void speech_service::handle_stt_engine_stopping(int task_id) {
    qDebug() << "stt engine stopping";

    if (current_task_id() == task_id) update_task_state();
}

void speech_service::handle_tts_engine_error(int task_id) {
    qDebug() << "tts engine error";

    emit error(error_t::tts_engine);

    if (current_task_id() == task_id) {
        cancel(task_id);
        if (m_stt_engine) {
            m_stt_engine.reset();
            qDebug() << "tts engine destroyed successfully";
        }
    }
}

void speech_service::handle_tts_engine_state_changed(tts_engine::state_t state,
                                                     int task_id) {
    qDebug() << "tts engine state changed:" << task_id;

    if ((state == tts_engine::state_t::stopped ||
         state == tts_engine::state_t::error) &&
        m_current_task && m_current_task->id == task_id) {
        if (state == tts_engine::state_t::error)
            emit error(error_t::tts_engine);

        stop_tts_engine();
    }

    emit requet_update_task_state();
}

void speech_service::handle_text_repair_engine_state_changed(
    text_repair_engine::state_t state, int task_id) {
    qDebug() << "text-repair engine state changed:" << task_id;

    if ((state == text_repair_engine::state_t::stopped ||
         state == text_repair_engine::state_t::error) &&
        m_current_task && m_current_task->id == task_id) {
        if (state == text_repair_engine::state_t::error)
            emit error(error_t::text_repair_engine);

        stop_text_repair_engine();
    }

    emit requet_update_task_state();
}

void speech_service::handle_mnt_engine_state_changed(mnt_engine::state_t state,
                                                     int task_id) {
    qDebug() << "mnt engine state changed:" << task_id;

    if ((state == mnt_engine::state_t::stopped ||
         state == mnt_engine::state_t::error) &&
        m_current_task && m_current_task->id == task_id) {
        stop_mnt_engine();
    }

    emit requet_update_task_state();
}

void speech_service::handle_mnt_translate_finished(
    const std::string &in_text, const std::string &in_lang,
    std::string &&out_text, const std::string &out_lang) {
    if (m_current_task) {
        emit mnt_translate_finished(
            QString::fromStdString(in_text), QString::fromStdString(in_lang),
            QString::fromStdString(out_text), QString::fromStdString(out_lang),
            m_current_task->id);
    }
}

void speech_service::handle_tts_speech_encoded(
    const std::string &text, const std::string &audio_file_path,
    tts_engine::audio_format_t format, double progress, bool last) {
    if (m_current_task) {
        emit tts_speech_encoded(
            {/*text=*/QString::fromStdString(text),
             /*audio_file_path=*/QString::fromStdString(audio_file_path),
             /*audio_format=*/format,
             /*remove_ausio_file=*/false,
             /*progress=*/progress,
             /*last=*/last,
             /*task_id=*/m_current_task->id});
    }
}

static QString file_ext_from_format(settings::audio_format_t format) {
    switch (format) {
        case settings::audio_format_t::AudioFormatWav:
            return "wav";
        case settings::audio_format_t::AudioFormatMp3:
            return "mp3";
        case settings::audio_format_t::AudioFormatOggVorbis:
            return "ogg";
        case settings::audio_format_t::AudioFormatOggOpus:
            return "opus";
        case settings::audio_format_t::AudioFormatAuto:
            break;
    }

    return "";
}

static QString audio_quality_to_str(settings::audio_quality_t quality) {
    switch (quality) {
        case settings::audio_quality_t::AudioQualityVbrHigh:
            return QStringLiteral("vbr_high");
        case settings::audio_quality_t::AudioQualityVbrMedium:
            return QStringLiteral("vbr_medium");
        case settings::audio_quality_t::AudioQualityVbrLow:
            return QStringLiteral("vbr_low");
    }

    return QStringLiteral("vbr_medium");
}

static media_compressor::format_t media_format_from_audio_format(
    settings::audio_format_t format) {
    switch (format) {
        case settings::audio_format_t::AudioFormatWav:
            return media_compressor::format_t::audio_wav;
        case settings::audio_format_t::AudioFormatMp3:
            return media_compressor::format_t::audio_mp3;
        case settings::audio_format_t::AudioFormatOggVorbis:
            return media_compressor::format_t::audio_ogg_vorbis;
        case settings::audio_format_t::AudioFormatOggOpus:
            return media_compressor::format_t::audio_ogg_opus;
        case settings::audio_format_t::AudioFormatAuto:
            break;
    }

    return media_compressor::format_t::unknown;
}

static media_compressor::quality_t media_quality_from_audio_quality(
    settings::audio_quality_t quality) {
    switch (quality) {
        case settings::audio_quality_t::AudioQualityVbrHigh:
            return media_compressor::quality_t::vbr_high;
        case settings::audio_quality_t::AudioQualityVbrMedium:
            return media_compressor::quality_t::vbr_medium;
        case settings::audio_quality_t::AudioQualityVbrLow:
            return media_compressor::quality_t::vbr_low;
    }

    return media_compressor::quality_t::vbr_medium;
}

static QString merged_file_path(const std::vector<QString> &files) {
    return QStringLiteral("%1/merged-%2")
        .arg(settings::instance()->cache_dir(),
             QString::number(qHash(std::accumulate(
                 files.cbegin(), files.cend(), QString{},
                 [](auto new_name, const auto &file) {
                     return std::move(new_name) + QFileInfo{file}.baseName();
                 }))));
}

void speech_service::handle_speech_to_file(const tts_partial_result_t &result) {
    if (m_current_task->id != result.task_id) {
        qWarning() << "invalid task:" << result.task_id;
        return;
    }

    m_current_task->progress = result.progress;
    if (!result.audio_file_path.isEmpty())
        m_current_task->files.push_back(result.audio_file_path);

    qDebug() << "partial speech to file progress:" << m_current_task->progress;

    emit tts_speech_to_file_progress_changed(m_current_task->progress,
                                             result.task_id);

    if (result.last) {
        qDebug() << "speech to file finished";

        if (tts_not_merge_files_options(m_current_task->options)) {
            QStringList files;
            std::transform(m_current_task->files.cbegin(),
                           m_current_task->files.cend(),
                           std::back_inserter(files),
                           [](const auto &file) { return file; });
            emit tts_speech_to_file_finished(std::move(files), result.task_id);
        } else {
            auto format =
                tts_audio_format_from_options(m_current_task->options);
            auto quality =
                tts_audio_quality_from_options(m_current_task->options);
            auto out_file = QStringLiteral("%1-%2.%3")
                                .arg(merged_file_path(m_current_task->files),
                                     audio_quality_to_str(quality),
                                     file_ext_from_format(format));

            qDebug() << "out file:" << out_file;

            if (!QFileInfo::exists(out_file)) {
                std::vector<std::string> input_files;
                std::transform(
                    m_current_task->files.cbegin(),
                    m_current_task->files.cend(),
                    std::back_inserter(input_files),
                    [](const auto &file) { return file.toStdString(); });

                bool error = false;

                try {
                    QEventLoop loop;
                    media_compressor compressor;

                    connect(
                        this, &speech_service::state_changed, &loop,
                        [this, &compressor]() {
                            if (state() !=
                                speech_service::state_t::writing_speech_to_file)
                                compressor.cancel();
                        });

                    media_compressor::options_t opts{
                        media_quality_from_audio_quality(quality),
                        media_compressor::flags_t::flag_none,
                        1.0,
                        {},
                        {}};

                    compressor.compress_to_file_async(
                        std::move(input_files), out_file.toStdString(),
                        media_format_from_audio_format(format), opts, {},
                        [&loop]() { loop.quit(); });

                    loop.exec();

                    error = compressor.error();

                } catch (const std::runtime_error &err) {
                    qWarning() << "compressor error:" << err.what();
                    error = true;
                }

                if (error) {
                    QFile::remove(out_file);
                    emit tts_engine_error(result.task_id);
                    cancel(result.task_id);
                    return;
                }
            }

            emit tts_speech_to_file_finished({out_file}, result.task_id);
        }

        cancel(result.task_id);
    }
}

void speech_service::handle_tts_speech_encoded(tts_partial_result_t result) {
    if (m_current_task && m_current_task->id == result.task_id) {
        if (m_current_task->speech_mode == speech_mode_t::play_speech) {
            m_tts_queue.push(std::move(result));
            handle_tts_queue();
        } else {
            handle_speech_to_file(std::move(result));
        }
    } else {
        qWarning() << "unknown task in tts speech encoded";
    }
}

void speech_service::play_beep(beep_role_t beep_role) {
    auto get_beep_file = [](const QString &name) {
        // get from user data dir
        auto file_user =
            QDir{QStandardPaths::writableLocation(QStandardPaths::DataLocation)}
                .filePath(name);
        qDebug() << "file user:" << file_user;
        if (QFileInfo::exists(file_user)) {
            return file_user;
        }

        // get from share dir
        auto file_sys = QStringLiteral("%1/%2").arg(
            module_tools::path_to_share_dir_for_path(name), name);
        qDebug() << "file sys:" << file_sys;
        if (QFileInfo::exists(file_sys)) {
            return file_sys;
        }

        return QString{};
    };

    qDebug() << "play beep:" << beep_role;

    auto beep_file = [&] {
        switch (beep_role) {
            case beep_role_t::stt_start_listen:
                return get_beep_file("beep-start-listen.wav");
            case beep_role_t::stt_end_listen:
                return get_beep_file("beep-end-listen.wav");
        }
        return QString{};
    }();

    if (beep_file.isEmpty()) {
        qWarning() << "can't find beep file";
        return;
    }

    m_beep_player.setMedia(QMediaContent{QUrl::fromLocalFile(beep_file)});
    m_beep_player.play();
}

void speech_service::handle_tts_queue() {
    if (m_tts_queue.empty()) return;

    if (m_player.state() == QMediaPlayer::State::PlayingState ||
        m_player.state() == QMediaPlayer::State::PausedState)
        return;

    if (m_current_task && m_current_task->paused) return;

    auto &result = m_tts_queue.front();

    if (!result.audio_file_path.isEmpty()) {
        if (result.audio_format != tts_engine::audio_format_t::wav) {
            auto audio_file_wav = result.audio_file_path + ".wav";
            media_compressor{}.decompress_to_file(
                {result.audio_file_path.toStdString()},
                audio_file_wav.toStdString(), {});
            result.audio_file_path = std::move(audio_file_wav);
            result.audio_format = tts_engine::audio_format_t::wav;
            result.remove_audio_file = true;
        }

        m_player.setMedia(
            QMediaContent{QUrl::fromLocalFile(result.audio_file_path)});

        m_player.play();

        emit tts_partial_speech_playing(result.text, result.task_id);
    } else if (result.last) {
        auto task = result.task_id;

        tts_stop_speech(task);
        if (m_tts_queue.empty()) {
            emit tts_partial_speech_playing("", task);
        } else {
            if (result.remove_audio_file) QFile::remove(result.audio_file_path);
            m_tts_queue.pop();
        }
        emit tts_play_speech_finished(task);
    } else {
        if (!m_tts_queue.empty()) {
            if (result.remove_audio_file) QFile::remove(result.audio_file_path);
            m_tts_queue.pop();
        }

        handle_tts_queue();
    }
}

void speech_service::handle_mnt_engine_error(mnt_engine::error_t error_type) {
    if (m_current_task) emit mnt_engine_error(error_type, m_current_task->id);
}

void speech_service::handle_mnt_engine_error(mnt_engine::error_t error_type,
                                             int task_id) {
    qDebug() << "mnt engine error";

    emit error([error_type]() {
        switch (error_type) {
            case mnt_engine::error_t::init:
                return error_t::mnt_engine;
            case mnt_engine::error_t::runtime:
                return error_t::mnt_runtime;
        }
        throw std::runtime_error("invalid mnt error");
    }());

    if (current_task_id() == task_id) {
        cancel(task_id);
        if (m_mnt_engine) {
            m_mnt_engine.reset();
            qDebug() << "mnt engine destroyed successfully";
        }
    }
}

void speech_service::handle_text_repair_engine_error(int task_id) {
    qDebug() << "text-repair engine error";

    if (current_task_id() == task_id) {
        cancel(task_id);
        if (m_text_repair_engine) {
            m_text_repair_engine.reset();
            qDebug() << "text-repair engine destroyed successfully";
        }
    }
}

void speech_service::handle_mnt_progress_changed(int task_id) {
    if (current_task_id() == task_id && m_mnt_engine) {
        emit mnt_translate_progress_changed(m_mnt_engine->progress(), task_id);
    }
}

void speech_service::handle_stt_text_decoded(const std::string &text,
                                             const std::string &lang) {
    if (m_current_task) {
        if (m_previous_task &&
            m_last_intermediate_text_task == m_previous_task->id) {
            emit stt_text_decoded(QString::fromStdString(text),
                                  QString::fromStdString(lang),
                                  m_previous_task->id);
        } else {
            emit stt_text_decoded(QString::fromStdString(text),
                                  QString::fromStdString(lang),
                                  m_current_task->id);
        }
    } else {
        qWarning() << "current task does not exist";
    }

    m_previous_task.reset();
}

void speech_service::handle_stt_engine_state_changed(
    [[maybe_unused]] stt_engine::speech_detection_status_t status,
    int task_id) {
    if (current_task_id() == task_id) update_task_state();
}

void speech_service::handle_ttt_text_repaired(const QString &text,
                                              int task_id) {
    if (current_task_id() == task_id) {
        emit ttt_repair_text_finished(text, task_id);

        stop_keepalive_current_task();
        stop_tts_engine();
        update_task_state();
    }
}

void speech_service::handle_player_state_changed(
    QMediaPlayer::State new_state) {
    qDebug() << "player new state:" << new_state;

    update_task_state();

    if (new_state == QMediaPlayer::State::StoppedState && m_current_task &&
        m_current_task->engine == engine_t::tts && !m_current_task->paused &&
        !m_tts_queue.empty()) {
        const auto &result = m_tts_queue.front();

        auto task = result.task_id;

        if (result.last) {
            tts_stop_speech(task);
            if (m_tts_queue.empty()) {
                emit tts_partial_speech_playing("", task);
            } else {
                if (result.remove_audio_file)
                    QFile::remove(result.audio_file_path);
                m_tts_queue.pop();
            }
            emit tts_play_speech_finished(task);
        } else {
            if (result.remove_audio_file) QFile::remove(result.audio_file_path);
            m_tts_queue.pop();

            if (m_tts_queue.empty()) {
                emit tts_partial_speech_playing("", task);
            }
        }

        handle_tts_queue();
    }
}

QVariantMap speech_service::available_models(
    const std::map<QString, model_data_t> &available_models_map) const {
    QVariantMap map;

    std::for_each(available_models_map.cbegin(), available_models_map.cend(),
                  [&map](const auto &p) {
                      map.insert(
                          p.first,
                          QStringList{p.second.model_id,
                                      QStringLiteral("%1 / %2").arg(
                                          p.second.name, p.second.lang_id),
                                      p.second.options, p.second.lang_id});
                  });

    return map;
}

QVariantMap speech_service::available_langs(
    const std::map<QString, model_data_t> &available_models_map) const {
    QVariantMap map;

    auto langs_map = models_manager::instance()->available_langs_map();

    std::for_each(
        available_models_map.cbegin(), available_models_map.cend(),
        [&](const auto &p) {
            const auto &lang_id = p.second.lang_id;
            if (!map.contains(lang_id) && langs_map.count(lang_id) != 0) {
                const auto &lang = langs_map.at(lang_id);
                if (lang_id == QStringLiteral("en")) {
                    map.insert(lang_id,
                               QStringList{p.second.model_id,
                                           QStringLiteral("%1 / %2").arg(
                                               lang.name, lang.id)});
                } else {
                    map.insert(lang_id,
                               QStringList{
                                   p.second.model_id,
                                   QStringLiteral("%1 (%2) / %3")
                                       .arg(lang.name, lang.name_en, lang.id)});
                }
            }
        });

    return map;
}

QVariantMap speech_service::available_trg_langs(
    const std::map<QString, model_data_t> &available_models_map) const {
    QVariantMap map;

    auto langs_map = models_manager::instance()->langs_map();

    std::for_each(
        available_models_map.cbegin(), available_models_map.cend(),
        [&](const auto &p) {
            const auto &lang_id = p.second.trg_lang_id;
            if (!map.contains(lang_id) && langs_map.count(lang_id) != 0) {
                const auto &lang = langs_map.at(lang_id);
                if (lang_id == QStringLiteral("en")) {
                    map.insert(lang_id,
                               QStringList{p.second.model_id,
                                           QStringLiteral("%1 / %2").arg(
                                               lang.name, lang.id)});
                } else {
                    map.insert(lang_id,
                               QStringList{
                                   p.second.model_id,
                                   QStringLiteral("%1 (%2) / %3")
                                       .arg(lang.name, lang.name_en, lang.id)});
                }
            }
        });

    return map;
}

QVariantList speech_service::available_lang_list(
    const std::map<QString, model_data_t> &available_models_map) const {
    QVariantList list;

    auto all_langs = models_manager::instance()->langs();

    std::set<QString> available_langs;

    std::for_each(
        available_models_map.cbegin(), available_models_map.cend(),
        [&](const auto &p) {
            if (available_langs.count(p.second.lang_id) == 0) {
                available_langs.insert(p.second.lang_id);

                auto it = std::find_if(
                    all_langs.cbegin(), all_langs.cend(),
                    [&](const auto &l) { return l.id == p.second.lang_id; });

                if (it == all_langs.cend()) return;

                if (p.second.lang_id == "en")
                    list.push_back(QStringList{
                        p.second.lang_id,
                        QStringLiteral("%1 / %2").arg(it->name, it->id)});
                else
                    list.push_back(
                        QStringList{p.second.lang_id,
                                    QStringLiteral("%1 (%2) / %3")
                                        .arg(it->name, it->name_en, it->id)});
            }
        });

    return list;
}

std::set<QString> speech_service::available_lang_set(
    const std::map<QString, model_data_t> &available_models_map) const {
    std::set<QString> available_langs;

    std::for_each(available_models_map.cbegin(), available_models_map.cend(),
                  [&](const auto &p) {
                      if (available_langs.count(p.second.lang_id) == 0) {
                          available_langs.insert(p.second.lang_id);
                      }
                  });

    return available_langs;
}

QVariantList speech_service::available_stt_tts_lang_list() const {
    QVariantList list;

    auto stt_langs = available_lang_set(m_available_stt_models_map);

    if (stt_langs.empty()) return list;

    auto tts_langs = available_lang_set(m_available_tts_models_map);

    if (tts_langs.empty()) return list;

    std::vector<QString> stt_tts_langs;
    std::set_intersection(stt_langs.cbegin(), stt_langs.cend(),
                          tts_langs.cbegin(), tts_langs.cend(),
                          std::back_inserter(stt_tts_langs));

    if (stt_langs.empty()) return list;

    auto all_langs = models_manager::instance()->langs();

    list.reserve(stt_tts_langs.size());

    for (auto &id : stt_tts_langs) {
        auto it = std::find_if(all_langs.cbegin(), all_langs.cend(),
                               [&](const auto &l) { return l.id == id; });

        if (it == all_langs.cend()) continue;

        list.push_back(
            QStringList{id, QStringLiteral("%1 / %2").arg(it->name, id)});
    }

    return list;
}

QVariantMap speech_service::available_stt_models() const {
    return available_models(m_available_stt_models_map);
}

QVariantMap speech_service::available_mnt_models() const {
    return available_models(m_available_mnt_models_map);
}

QVariantMap speech_service::available_tts_models() const {
    return available_models(m_available_tts_models_map);
}

QVariantMap speech_service::available_ttt_models() const {
    return available_models(m_available_ttt_models_map);
}

QVariantMap speech_service::available_stt_langs() const {
    return available_langs(m_available_stt_models_map);
}

QVariantList speech_service::available_stt_lang_list() const {
    return available_lang_list(m_available_stt_models_map);
}

QVariantMap speech_service::available_mnt_langs() const {
    return available_langs(m_available_mnt_models_map);
}

QVariantList speech_service::available_mnt_lang_list() const {
    return available_lang_list(m_available_mnt_models_map);
}

QVariantMap speech_service::available_tts_langs() const {
    return available_langs(m_available_tts_models_map);
}

QVariantList speech_service::available_tts_lang_list() const {
    return available_lang_list(m_available_tts_models_map);
}

QVariantMap speech_service::available_ttt_langs() const {
    return available_langs(m_available_ttt_models_map);
}

void speech_service::download_model(const QString &id) {
    models_manager::instance()->download_model(id);
}

void speech_service::delete_model(const QString &id) {
    if (m_current_task && m_current_task->model_id == id) stop_stt();
    models_manager::instance()->delete_model(id);
}

void speech_service::handle_audio_available() {
    if (m_source && m_stt_engine && m_stt_engine->started()) {
        auto status = m_stt_engine->speech_detection_status();
        if (m_source->type() == audio_source::source_type::mic) {
            if ((status == stt_engine::speech_detection_status_t::decoding &&
                 m_current_task &&
                 (m_current_task->flags &
                  task_flags_stt_clear_mic_audio_when_decoding)) ||
                status == stt_engine::speech_detection_status_t::initializing) {
                m_source->clear();
                return;
            }
        } else if (status ==
                   stt_engine::speech_detection_status_t::initializing) {
            m_source->slowdown();
            return;
        }

        auto [buf, max_size] = m_stt_engine->borrow_buf();

        if (buf) {
            auto audio_data = m_source->read_audio(buf, max_size);

            m_stt_engine->return_buf(buf, audio_data.size, audio_data.sof,
                                     audio_data.eof);
            set_progress(m_source->progress());

            if (audio_data.eof)
                m_source->slowdown();
            else
                m_source->speedup();
        } else {
            m_source->slowdown();
        }
    }
}

void speech_service::set_progress(double p) {
    if (audio_source_type() == source_t::file && m_current_task) {
        const auto delta = p - m_progress;
        if (delta < 0.0 || delta > 0.01 || p < 0.0 || p >= 1.0) {
            m_progress = p;
            emit stt_transcribe_file_progress_changed(m_progress,
                                                      m_current_task->id);
        }
    }
}

double speech_service::stt_transcribe_file_progress(int task) const {
    if (audio_source_type() == source_t::file) {
        if (m_current_task && m_current_task->id == task) {
            return m_progress;
        }
        qWarning() << "invalid task id";
    }

    return -1.0;
}

double speech_service::tts_speech_to_file_progress(int task) const {
    if (m_current_task && m_current_task->id == task &&
        m_current_task->speech_mode == speech_mode_t::speech_to_file) {
        return m_progress;
    }

    qWarning() << "invalid task id";

    return -1.0;
}

double speech_service::mnt_translate_progress(int task) const {
    if (m_current_task && m_current_task->id == task && m_mnt_engine) {
        return m_mnt_engine->progress();
    }

    qWarning() << "invalid task id";

    return -1.0;
}

QVariantMap speech_service::mnt_out_langs(QString in_lang) const {
    QVariantMap map;

    in_lang = in_lang.toLower();

    std::map<QString, model_data_t> available_models_map;

    bool search_for_en_trg = in_lang != QStringLiteral("en");
    bool found_en_trg = false;

    std::for_each(m_available_mnt_models_map.cbegin(),
                  m_available_mnt_models_map.cend(), [&](const auto &pair) {
                      if (pair.second.lang_id == in_lang) {
                          available_models_map.insert(pair);

                          if (search_for_en_trg && !found_en_trg)
                              found_en_trg = pair.second.trg_lang_id ==
                                             QStringLiteral("en");
                      }
                  });

    if (found_en_trg) {
        std::for_each(m_available_mnt_models_map.cbegin(),
                      m_available_mnt_models_map.cend(), [&](const auto &pair) {
                          if (pair.second.lang_id == QStringLiteral("en") &&
                              pair.second.trg_lang_id != in_lang) {
                              available_models_map.insert(pair);
                          }
                      });
    }

    if (available_models_map.empty()) return map;

    map = available_trg_langs(available_models_map);

    return map;
}

QVariantMap speech_service::features_availability() {
    if (m_features_availability.isEmpty()) {
        auto py_availability = py_executor::instance()->libs_availability;
        if (py_availability) {
            qDebug() << "features availability ready";
            unsigned int hw_feature_flags =
                settings::hw_feature_flags_t::hw_feature_none;

            m_features_availability.insert(
                "coqui-tts",
                QVariantList{py_availability->coqui_tts, "Coqui TTS"});
            m_features_availability.insert(
                "whisperspeech-tts",
                QVariantList{py_availability->whisperspeech_tts,
                             "WhisperSpeech TTS"});
            m_features_availability.insert(
                "parler-tts",
                QVariantList{py_availability->parler_tts, "Parler-TTS"});
            m_features_availability.insert(
                "f5-tts", QVariantList{py_availability->f5_tts, "F5-TTS"});
            m_features_availability.insert(
                "kokoro-tts",
                QVariantList{py_availability->kokoro_tts, "Kokoro TTS"});
            m_features_availability.insert(
                "kokoro-tts-ja", QVariantList{py_availability->kokoro_tts &&
                                                  py_availability->kokoro_ja,
                                              "Kokoro TTS " + tr("Japanese")});
            m_features_availability.insert(
                "kokoro-tts-zh", QVariantList{py_availability->kokoro_tts &&
                                                  py_availability->kokoro_zh,
                                              "Kokoro TTS " + tr("Chinese")});
#ifdef ARCH_X86_64
            auto has_cuda = gpu_tools::has_cuda_runtime();
            auto has_cudnn = gpu_tools::has_cudnn();
            auto has_hip = gpu_tools::has_hip();

            bool tts_coqui_cuda =
                py_availability->coqui_tts && py_availability->torch_cuda;
            bool tts_coqui_hip =
                py_availability->coqui_tts && py_availability->torch_hip;
            m_features_availability.insert(
                "coqui-tts-cuda",
                QVariantList{tts_coqui_cuda,
                             "Coqui TTS CUDA " + tr("HW acceleration")});
            m_features_availability.insert(
                "coqui-tts-hip",
                QVariantList{tts_coqui_hip,
                             "Coqui TTS ROCm " + tr("HW acceleration")});
            if (tts_coqui_cuda)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_coqui_cuda;
            if (tts_coqui_hip)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_coqui_hip;

            bool tts_whisperspeech_cuda = py_availability->whisperspeech_tts &&
                                          py_availability->torch_cuda;
            bool tts_whisperspeech_hip = py_availability->whisperspeech_tts &&
                                         py_availability->torch_hip;
            m_features_availability.insert(
                "whisperspeech-tts-cuda",
                QVariantList{
                    tts_whisperspeech_cuda,
                    "WhisperSpeech TTS CUDA " + tr("HW acceleration")});
            m_features_availability.insert(
                "whisperspeech-tts-hip",
                QVariantList{tts_whisperspeech_hip, "WhisperSpeech TTS ROCm " +
                                                        tr("HW acceleration")});
            if (tts_whisperspeech_cuda)
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_tts_whisperspeech_cuda;
            if (tts_whisperspeech_hip)
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_tts_whisperspeech_hip;

            bool tts_parler_cuda =
                py_availability->parler_tts && py_availability->torch_cuda;
            bool tts_parler_hip =
                py_availability->parler_tts && py_availability->torch_hip;
            m_features_availability.insert(
                "parler-tts-cuda",
                QVariantList{tts_parler_cuda,
                             "Parler-TTS CUDA " + tr("HW acceleration")});
            m_features_availability.insert(
                "parler-tts-hip",
                QVariantList{tts_parler_hip,
                             "Parler-TTS ROCm " + tr("HW acceleration")});
            if (tts_parler_cuda)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_parler_cuda;
            if (tts_parler_hip)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_parler_hip;

            bool tts_f5_cuda =
                py_availability->f5_tts && py_availability->torch_cuda;
            bool tts_f5_hip =
                py_availability->f5_tts && py_availability->torch_hip;
            m_features_availability.insert(
                "f5-tts-cuda",
                QVariantList{tts_f5_cuda,
                             "F5-TTS CUDA " + tr("HW acceleration")});
            m_features_availability.insert(
                "f5-tts-hip",
                QVariantList{tts_f5_hip,
                             "F5-TTS ROCm " + tr("HW acceleration")});
            if (tts_f5_cuda)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_f5_cuda;
            if (tts_f5_hip)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_f5_hip;

            bool tts_kokoro_cuda =
                py_availability->kokoro_tts && py_availability->torch_cuda;
            bool tts_kokoro_hip =
                py_availability->kokoro_tts && py_availability->torch_hip;
            m_features_availability.insert(
                "kokoro-tts-cuda",
                QVariantList{tts_kokoro_cuda,
                             "Kokoro TTS CUDA " + tr("HW acceleration")});
            m_features_availability.insert(
                "kokoro-tts-hip",
                QVariantList{tts_kokoro_hip,
                             "Kokoro TTS ROCm " + tr("HW acceleration")});
            if (tts_kokoro_cuda)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_kokoro_cuda;
            if (tts_kokoro_hip)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_tts_kokoro_hip;
#endif
            m_features_availability.insert(
                "coqui-tts-ja", QVariantList{py_availability->coqui_tts &&
                                                 py_availability->mecab,
                                             "Coqui TTS " + tr("Japanese")});
            m_features_availability.insert(
                "coqui-tts-ko", QVariantList{py_availability->coqui_tts &&
                                                 py_availability->uroman,
                                             "Coqui TTS " + tr("Korean")});
            m_features_availability.insert(
                "mimic3-tts",
                QVariantList{py_availability->mimic3_tts, "Mimic3 TTS"});
            m_features_availability.insert(
                "mimic3-tts-de", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_de,
                                              "Mimic3 TTS " + tr("German")});
            m_features_availability.insert(
                "mimic3-tts-es", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_es,
                                              "Mimic3 TTS " + tr("Spanish")});
            m_features_availability.insert(
                "mimic3-tts-fr", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_fr,
                                              "Mimic3 TTS " + tr("French")});
            m_features_availability.insert(
                "mimic3-tts-it", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_it,
                                              "Mimic3 TTS " + tr("Italian")});
            m_features_availability.insert(
                "mimic3-tts-ru", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_ru,
                                              "Mimic3 TTS " + tr("Russian")});
            m_features_availability.insert(
                "mimic3-tts-sw", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_sw,
                                              "Mimic3 TTS " + tr("Swahili")});
            m_features_availability.insert(
                "mimic3-tts-fa", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_fa,
                                              "Mimic3 TTS " + tr("Persian")});
            m_features_availability.insert(
                "mimic3-tts-nl", QVariantList{py_availability->mimic3_tts &&
                                                  py_availability->gruut_nl,
                                              "Mimic3 TTS " + tr("Dutch")});
            m_features_availability.insert(
                "faster-whisper-stt",
                QVariantList{py_availability->faster_whisper,
                             "FasterWhisper STT"});
#ifdef ARCH_X86_64
            bool stt_fasterwhisper_cuda = py_availability->faster_whisper &&
                                          py_availability->ctranslate2_cuda &&
                                          has_cuda && has_cudnn;
            bool stt_fasterwhisper_hip = py_availability->faster_whisper &&
                                         py_availability->ctranslate2_cuda &&
                                         has_hip;
            stt_fasterwhisper_hip =
                false;  // right now, ct2 doesn't support hip
            m_features_availability.insert(
                "faster-whisper-stt-cuda",
                QVariantList{
                    stt_fasterwhisper_cuda,
                    "FasterWhisper STT CUDA " + tr("HW acceleration")});
            // consider this: https://github.com/arlo-phoenix/CTranslate2-rocm
            /*m_features_availability.insert(
                "faster-whisper-stt-hip",
                QVariantList{stt_fasterwhisper_hip, "FasterWhisper
                STT ROCm " + tr("HW acceleration")});*/
            if (stt_fasterwhisper_cuda) {
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_stt_fasterwhisper_cuda;
            }
            if (stt_fasterwhisper_hip) {
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_stt_fasterwhisper_hip;
            }
#endif
            m_features_availability.insert(
                "punctuator", QVariantList{py_availability->transformers,
                                           tr("Punctuation restoration")});
            m_features_availability.insert(
                "diacritizer-he",
                QVariantList{
                    py_availability->transformers && py_availability->unikud,
                    tr("Diacritics restoration for Hebrew")});

            bool stt_ds = ds_engine::available();
            bool stt_whispercpp = whisper_engine::available();
            bool stt_whispercpp_vulkan = whisper_engine::has_vulkan();
            bool mnt = mnt_engine::available();

            m_features_availability.insert(
                "coqui-stt", QVariantList{stt_ds, "Coqui/DeepSpeech STT"});
            m_features_availability.insert("translator",
                                           QVariantList{mnt, "Translator"});
            m_features_availability.insert(
                "whispercpp-stt",
                QVariantList{stt_whispercpp, "WhisperCpp STT "});
            m_features_availability.insert(
                "whispercpp-stt-vulkan",
                QVariantList{stt_whispercpp_vulkan,
                             "WhisperCpp STT Vulkan " + tr("HW acceleration")});
            if (stt_whispercpp_vulkan)
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_stt_whispercpp_vulkan;
#ifdef ARCH_X86_64
            bool stt_whispercpp_cuda = whisper_engine::has_cuda();
            m_features_availability.insert(
                "whispercpp-stt-cuda",
                QVariantList{whisper_engine::has_cuda(),
                             "WhisperCpp STT CUDA " + tr("HW acceleration")});
            if (stt_whispercpp_cuda)
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_stt_whispercpp_cuda;

            bool stt_whispercpp_hip = whisper_engine::has_hip();
            m_features_availability.insert(
                "whispercpp-stt-hip",
                QVariantList{stt_whispercpp_hip,
                             "WhisperCpp STT ROCm " + tr("HW acceleration")});
            if (stt_whispercpp_hip)
                hw_feature_flags |=
                    settings::hw_feature_flags_t::hw_feature_stt_whispercpp_hip;

            bool stt_whispercpp_openvino = whisper_engine::has_openvino();
            m_features_availability.insert(
                "whispercpp-stt-openvino",
                QVariantList{
                    stt_whispercpp_openvino,
                    "WhisperCpp STT OpenVINO " + tr("HW acceleration")});
            if (stt_whispercpp_openvino)
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_stt_whispercpp_openvino;

            bool stt_whispercpp_opencl = whisper_engine::has_opencl();
            m_features_availability.insert(
                "whispercpp-stt-opencl",
                QVariantList{stt_whispercpp_opencl,
                             "WhisperCpp STT OpenCL " + tr("HW acceleration")});
            if (stt_whispercpp_opencl)
                hw_feature_flags |= settings::hw_feature_flags_t::
                    hw_feature_stt_whispercpp_opencl;
#endif
            auto tts_rhvoice = rhvoice_engine::available();
            m_features_availability.insert(
                "rhvoice-tts", QVariantList{tts_rhvoice, "RHVoice TTS"});

            auto stt_vosk = vosk_engine::available();
            m_features_availability.insert("vosk-stt",
                                           QVariantList{stt_vosk, "Vosk STT"});

            models_manager::instance()->update_models_using_availability(
                {/*tts_coqui=*/py_availability->coqui_tts,
                 /*tts_mimic3=*/py_availability->mimic3_tts,
                 /*tts_mimic3_de=*/py_availability->mimic3_tts &&
                     py_availability->gruut_de,
                 /*tts_mimic3_es=*/py_availability->mimic3_tts &&
                     py_availability->gruut_es,
                 /*tts_mimic3_fr=*/py_availability->mimic3_tts &&
                     py_availability->gruut_fr,
                 /*tts_mimic3_it=*/py_availability->mimic3_tts &&
                     py_availability->gruut_it,
                 /*tts_mimic3_ru=*/py_availability->mimic3_tts &&
                     py_availability->gruut_ru,
                 /*tts_mimic3_sw=*/py_availability->mimic3_tts &&
                     py_availability->gruut_sw,
                 /*tts_mimic3_fa=*/py_availability->mimic3_tts &&
                     py_availability->gruut_fa,
                 /*tts_mimic3_nl=*/py_availability->mimic3_tts &&
                     py_availability->gruut_nl,
                 /*tts_rhvoice=*/tts_rhvoice,
                 /*tts_whisperspeech=*/py_availability->whisperspeech_tts,
                 /*tts_parler=*/py_availability->parler_tts,
                 /*tts_f5=*/py_availability->f5_tts,
                 /*tts_kokoro=*/py_availability->kokoro_tts,
                 /*tts_kokoro_ja=*/py_availability->kokoro_ja,
                 /*tts_kokoro_zh=*/py_availability->kokoro_zh,
                 /*stt_fasterwhisper=*/py_availability->faster_whisper,
                 /*stt_ds=*/stt_ds,
                 /*stt_vosk=*/stt_vosk,
                 /*stt_whispercpp=*/stt_whispercpp,
                 /*mnt_bergamot=*/mnt,
                 /*ttt_hftc=*/py_availability->transformers,
                 /*option_r=*/py_availability->uroman});

            settings::instance()->scan_hw_devices(hw_feature_flags);

            m_features_availability.insert(
                "whispercpp-gpu-devices",
                variant_list_from_list(
                    settings::instance()->whispercpp_gpu_devices()));
            m_features_availability.insert(
                "fasterwhisper-gpu-devices",
                variant_list_from_list(
                    settings::instance()->fasterwhisper_gpu_devices()));
            m_features_availability.insert(
                "coqui-gpu-devices",
                variant_list_from_list(
                    settings::instance()->coqui_gpu_devices()));
            m_features_availability.insert(
                "whisperspeech-gpu-devices",
                variant_list_from_list(
                    settings::instance()->whisperspeech_gpu_devices()));
            m_features_availability.insert(
                "parler-gpu-devices",
                variant_list_from_list(
                    settings::instance()->parler_gpu_devices()));
            m_features_availability.insert(
                "f5-gpu-devices",
                variant_list_from_list(settings::instance()->f5_gpu_devices()));
            m_features_availability.insert(
                "kokoro-gpu-devices",
                variant_list_from_list(
                    settings::instance()->kokoro_gpu_devices()));

            m_features_availability.insert(
                "addon-flags",
                QVariantList{} << settings::instance()->addon_flags());
            m_features_availability.insert(
                "system-flags",
                QVariantList{} << settings::instance()->system_flags());
            m_features_availability.insert(
                "error-flags",
                QVariantList{} << settings::instance()->error_flags());

            m_features_availability.insert(
                "py-version",
                QVariantList{} << QString::fromStdString(
                    fmt::format("{}.{}.{}", py_availability->py_version.major,
                                py_availability->py_version.minor,
                                py_availability->py_version.micro)));
            refresh_status();

            emit features_availability_updated();
        } else {
            qDebug() << "delaying features availability";
            m_features_availability_timer.start();
        }
    }

    return m_features_availability;
}

int speech_service::next_task_id() {
    m_last_task_id = (m_last_task_id + 1) % std::numeric_limits<int>::max();
    return m_last_task_id;
}

static int stream_index_from_options(const QVariantMap &options) {
    if (auto k = QStringLiteral("stream_index"); options.contains(k)) {
        bool ok = false;
        auto value = options.value(k).toInt(&ok);
        if (ok) return value;
    }

    return -1;
}

int speech_service::stt_transcribe_file(const QString &file, QString lang,
                                        QString out_lang,
                                        const QVariantMap &options) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot transcribe_file, invalid state";
        return INVALID_TASK;
    }

    if (file.isEmpty()) {
        qWarning() << "cannot transcribe_file, file not provided";
        emit error(error_t::file_source);
        return INVALID_TASK;
    }

    if (lang.contains('-')) lang = lang.split('-').first();
    if (out_lang.contains('-')) out_lang = out_lang.split('-').first();

    qDebug() << "stt transcribe file";

    m_current_task.reset();

    m_current_task = {
        next_task_id(),
        engine_t::stt,
        restart_stt_engine(speech_mode_t::automatic, lang, out_lang, options),
        speech_mode_t::automatic,
        out_lang,
        0.0,
        {},
        options,
        false,
        task_flags_none};

    if (m_current_task->model_id.isEmpty()) {
        m_current_task.reset();

        qWarning() << "failed to restart engine";

        emit current_task_changed();

        refresh_status();

        return INVALID_TASK;
    }

    auto stream_index = stream_index_from_options(options);

    qDebug() << "requested stream index:" << stream_index;

    try {
        if (QFileInfo::exists(file))
            restart_audio_source(stt_source_file_props_t{file, stream_index});
        else
            restart_audio_source(stt_source_file_props_t{
                QUrl{file}.toLocalFile(), stream_index});
    } catch (const std::runtime_error &err) {
        m_current_task.reset();
        qCritical() << "audio source error:" << err.what();
        emit current_task_changed();
        refresh_status();
        emit error(QFileInfo::exists(file) ? error_t::file_source
                                           : error_t::mic_source);
        return INVALID_TASK;
    }

    start_keepalive_current_task();

    emit current_task_changed();

    update_task_state();

    return m_current_task->id;
}

int speech_service::mnt_translate(const QString &text, QString lang,
                                  QString out_lang,
                                  const QVariantMap &options) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot mnt translate, invalid state";
        return INVALID_TASK;
    }

    if (lang.contains('-')) lang = lang.split('-').first();
    if (lang.contains('-')) out_lang = out_lang.split('-').first();

    if (m_current_task) {
        if (m_current_task->engine == engine_t::stt)
            stt_stop_listen(m_current_task->id);
        else if (m_current_task->engine == engine_t::tts)
            tts_stop_speech(m_current_task->id);
    }

    qDebug() << "mnt translate";

    m_current_task.reset();

    m_current_task = {next_task_id(),
                      engine_t::mnt,
                      restart_mnt_engine(lang, out_lang, options),
                      speech_mode_t::translate,
                      out_lang,
                      0.0,
                      {},
                      options,
                      false,
                      task_flags_none};

    if (m_current_task->model_id.isEmpty()) {
        m_current_task.reset();

        qWarning() << "failed to restart engine";

        emit current_task_changed();

        refresh_status();

        return INVALID_TASK;
    }

    if (m_mnt_engine) m_mnt_engine->translate(text.toStdString());

    start_keepalive_current_task();

    emit current_task_changed();

    update_task_state();

    return m_current_task->id;
}

static text_repair_engine::task_type_t text_repair_task_type_from_options(
    const QVariantMap &options) {
    if (options.contains(QStringLiteral("task_type"))) {
        bool ok = false;
        auto value = options.value(QStringLiteral("task_type")).toInt(&ok);
        if (ok) return static_cast<text_repair_engine::task_type_t>(value);
    }
    return text_repair_engine::task_type_t::none;
}

int speech_service::ttt_repair_text(const QString &text,
                                    const QVariantMap &options) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot tts repair text, invalid state";
        return INVALID_TASK;
    }

    if (m_current_task) {
        if (m_current_task->engine == engine_t::stt) {
            stt_stop_listen(m_current_task->id);
        } else if (m_current_task->engine == engine_t::tts) {
            tts_stop_speech(m_current_task->id);
        }
    }

    qDebug() << "ttt repair text";

    m_current_task.reset();

    if (!restart_text_repair_engine(options)) {
        m_current_task.reset();
        qWarning() << "failed to restart engine";
        emit current_task_changed();
        refresh_status();
        return INVALID_TASK;
    }

    m_current_task = {
        next_task_id(), engine_t::text_repair,
        /*model=*/{},   speech_mode_t::play_speech, {}, 0.0, {}, options, false,
        task_flags_none};

    if (m_text_repair_engine) {
        auto task_type = text_repair_task_type_from_options(options);
        if (task_type == text_repair_engine::task_type_t::none) {
            m_current_task.reset();
            qWarning() << "invalid text repair task type";
            emit current_task_changed();
            refresh_status();
            return INVALID_TASK;
        }

        m_text_repair_engine->repair_text(text.toStdString(), task_type);
    }

    start_keepalive_current_task();

    emit current_task_changed();

    update_task_state();

    return m_current_task->id;
}

int speech_service::stt_start_listen(speech_mode_t mode, QString lang,
                                     QString out_lang,
                                     const QVariantMap &options) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot stt start listen, invalid state";
        return INVALID_TASK;
    }

    if (lang.contains('-')) lang = lang.split('-').first();
    if (out_lang.contains('-')) out_lang = out_lang.split('-').first();

    if (m_current_task && (state() == state_t::listening_auto ||
                           state() == state_t::listening_manual ||
                           state() == state_t::listening_single_sentence)) {
        qDebug() << "ignoring start listen because already listening:"
                 << state();
        return m_current_task->id;
    }

    qDebug() << "stt start listen";

    m_current_task.reset();

    std::underlying_type_t<task_flags_t> flags = task_flags_none;
    if (get_bool_value_from_options("stt_clear_mic_audio_when_decoding", false,
                                    options)) {
        flags |= task_flags_stt_clear_mic_audio_when_decoding;
    }
    if (get_bool_value_from_options("stt_play_beep", false, options)) {
        flags |= task_flags_stt_play_beep;
    }

    m_current_task = {next_task_id(),
                      engine_t::stt,
                      restart_stt_engine(mode, lang, out_lang, options),
                      mode,
                      out_lang,
                      0.0,
                      {},
                      options,
                      false,
                      flags};

    if (m_current_task->model_id.isEmpty()) {
        m_current_task.reset();

        qWarning() << "failed to restart engine";

        emit current_task_changed();

        refresh_status();

        return INVALID_TASK;
    }

    restart_audio_source([&] {
        if (options.contains(QStringLiteral("audio_input")))
            return options.value(QStringLiteral("audio_input")).toString();
        return QString{};
    }());
    if (m_stt_engine) m_stt_engine->set_speech_started(true);

    start_keepalive_current_task();

    emit current_task_changed();

    update_task_state();

    return m_current_task->id;
}

int speech_service::tts_play_speech(const QString &text, QString lang,
                                    const QVariantMap &options) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot tts play speech, invalid state";
        return INVALID_TASK;
    }

    if (lang.contains('-')) lang = lang.split('-').first();

    if (m_current_task) {
        if (m_current_task->engine == engine_t::stt) {
            stt_stop_listen(m_current_task->id);
        } else if (m_current_task->engine == engine_t::tts)
            tts_stop_speech(m_current_task->id);
    }

    qDebug() << "tts play speech";

    m_current_task.reset();

    m_current_task = {next_task_id(),
                      engine_t::tts,
                      restart_tts_engine(lang, options),
                      speech_mode_t::play_speech,
                      lang,
                      0.0,
                      {},
                      options,
                      false,
                      task_flags_none};

    if (m_current_task->model_id.isEmpty()) {
        m_current_task.reset();

        qWarning() << "failed to restart engine";

        emit current_task_changed();

        refresh_status();

        return INVALID_TASK;
    }

    if (m_stt_engine) m_stt_engine->stop();
    restart_audio_source({});

    if (m_tts_engine) m_tts_engine->encode_speech(text.toStdString());

    start_keepalive_current_task();

    emit current_task_changed();

    update_task_state();

    return m_current_task->id;
}

int speech_service::tts_speech_to_file(const QString &text, QString lang,
                                       const QVariantMap &options) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot tts speech to file, invalid state";
        return INVALID_TASK;
    }

    if (lang.contains('-')) lang = lang.split('-').first();

    if (m_current_task) {
        if (m_current_task->engine == engine_t::stt) {
            stt_stop_listen(m_current_task->id);
        } else if (m_current_task->engine == engine_t::tts)
            tts_stop_speech(m_current_task->id);
    }

    qDebug() << "tts speech to file";

    m_current_task.reset();

    m_current_task = {next_task_id(),
                      engine_t::tts,
                      restart_tts_engine(lang, options),
                      speech_mode_t::speech_to_file,
                      lang,
                      0.0,
                      {},
                      options,
                      false,
                      task_flags_none};

    if (m_current_task->model_id.isEmpty()) {
        m_current_task.reset();

        qWarning() << "failed to restart engine";

        emit current_task_changed();

        refresh_status();

        return INVALID_TASK;
    }

    if (m_tts_engine) m_tts_engine->encode_speech(text.toStdString());

    start_keepalive_current_task();

    emit current_task_changed();

    update_task_state();

    return m_current_task->id;
}

int speech_service::cancel(int task) {
    if (state() == state_t::unknown) {
        qWarning() << "cannot cancel, invalid state";
        return FAILURE;
    }

    qDebug() << "cancel:" << task;

    if (!m_current_task) {
        qWarning() << "no current task";
        return FAILURE;
    }

    if (m_current_task->id != task) {
        qWarning() << "invalid task id";
    }

    stop_keepalive_current_task();

    m_player.pause();

    if (m_current_task->engine == engine_t::tts) {
        if (m_tts_engine &&
            (m_tts_engine->state() == tts_engine::state_t::speech_encoding ||
             m_tts_engine->state() == tts_engine::state_t::initializing))
            m_tts_engine->request_stop();
        else
            stop_tts_engine();
    } else if (m_current_task->engine == engine_t::stt) {
        if (m_stt_engine && m_stt_engine->started())
            m_stt_engine->request_stop();
        else
            stop_stt_engine();
    } else if (m_current_task->engine == engine_t::mnt) {
        if (m_mnt_engine &&
            m_mnt_engine->state() == mnt_engine::state_t::translating)
            m_mnt_engine->request_stop();
        else
            stop_mnt_engine();
    }

    clean_tts_queue();

    m_player.stop();

    refresh_status();

    return SUCCESS;
}

int speech_service::stt_stop_listen(int task) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot stop_listen, invalid state";
        return FAILURE;
    }

    qDebug() << "stt stop listen";

    if (audio_source_type() == source_t::mic) {
        if (m_current_task && m_current_task->id == task) {
            if (m_current_task->engine != engine_t::stt) {
                qWarning() << "valid task id but invalid engine";
                return FAILURE;
            }

            stop_keepalive_current_task();

            if (m_stt_engine && m_stt_engine->started()) {
                stop_stt_engine_gracefully();
            } else {
                stop_stt_engine();
            }

        } else {
            qWarning() << "invalid task id";
            return FAILURE;
        }
    } else {
        if (m_current_task && m_current_task->id == task &&
            m_current_task->engine != engine_t::stt) {
            qWarning() << "valid task id but invalid engine";
            return FAILURE;
        }
    }

    return SUCCESS;
}

int speech_service::tts_pause_speech(int task) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot pause speech invalid state";
        return FAILURE;
    }

    if (!m_current_task || m_current_task->id != task) {
        qWarning() << "invalid task id";
        return FAILURE;
    }

    if (m_current_task->engine != engine_t::tts) {
        qWarning() << "valid task id but invalid engine";
        return FAILURE;
    }

    qDebug() << "pausing speech";

    m_current_task->paused = true;

    if (m_player.state() == QMediaPlayer::PlayingState) m_player.pause();

    update_task_state();

    return SUCCESS;
}

int speech_service::tts_resume_speech(int task) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot resume speech invalid state";
        return FAILURE;
    }

    if (!m_current_task || m_current_task->id != task) {
        qWarning() << "invalid task id";
        return FAILURE;
    }

    if (m_current_task->engine != engine_t::tts) {
        qWarning() << "valid task id but invalid engine";
        return FAILURE;
    }

    if (!m_current_task->paused) {
        qWarning() << "player not in paused state";
        return FAILURE;
    }

    qDebug() << "reasuming speech";

    m_current_task->paused = false;

    if (m_player.state() == QMediaPlayer::PausedState) m_player.play();

    handle_tts_queue();

    update_task_state();

    return SUCCESS;
}

void speech_service::clean_tts_queue() {
    while (!m_tts_queue.empty()) {
        if (m_tts_queue.front().remove_audio_file)
            QFile::remove(m_tts_queue.front().audio_file_path);
        m_tts_queue.pop();
    }
}

int speech_service::tts_stop_speech(int task) {
    if (state() == state_t::unknown || state() == state_t::not_configured ||
        state() == state_t::busy) {
        qWarning() << "cannot stop speech, invalid state";
        return FAILURE;
    }

    if (!m_current_task || m_current_task->id != task) {
        qWarning() << "invalid task id";
        return FAILURE;
    }

    if (m_current_task->engine != engine_t::tts) {
        qWarning() << "valid task id but invalid engine";
        return FAILURE;
    }

    stop_keepalive_current_task();

    m_player.pause();
    stop_tts_engine();

    clean_tts_queue();

    m_player.stop();

    return SUCCESS;
}

void speech_service::stop_stt_engine_gracefully() {
    qDebug() << "stop stt engine gracefully";

    if (m_source) {
        if (m_stt_engine) m_stt_engine->set_speech_started(false);
        if (!m_source->stopped()) {
            m_source->stop();
            if (m_current_task &&
                (m_current_task->flags & task_flags_stt_play_beep)) {
                play_beep(beep_role_t::stt_end_listen);
            }
        }
    } else {
        stop_stt_engine();
    }
}

void speech_service::stop_tts_engine() {
    qDebug() << "stop tts engine";

    if (m_current_task) {
        m_current_task.reset();
        stop_keepalive_current_task();
        emit current_task_changed();
    }

    if (m_tts_engine) m_tts_engine->request_stop();

    update_task_state();
}

void speech_service::stop_text_repair_engine() {
    qDebug() << "stop text-repair engine";

    if (m_current_task) {
        m_current_task.reset();
        stop_keepalive_current_task();
        emit current_task_changed();
    }

    if (m_text_repair_engine) m_text_repair_engine->request_stop();

    update_task_state();
}

void speech_service::stop_stt_engine() {
    qDebug() << "stop stt engine";

    if (m_stt_engine) m_stt_engine->stop();

    restart_audio_source({});

    if (m_current_task && m_current_task->engine == engine_t::stt) {
        m_current_task.reset();
        stop_keepalive_current_task();
        emit current_task_changed();
    }

    update_task_state();
}

void speech_service::stop_mnt_engine() {
    qDebug() << "stop mnt engine";

    if (m_mnt_engine) m_mnt_engine->stop();

    if (m_current_task) {
        m_current_task.reset();
        stop_keepalive_current_task();
        emit current_task_changed();
    }

    update_task_state();
}

void speech_service::stop_stt() {
    qDebug() << "stop stt";
    stop_stt_engine();
}

void speech_service::handle_audio_error() {
    if (audio_source_type() == source_t::file && m_current_task) {
        qWarning() << "file audio source error";
        emit error(error_t::file_source);
        cancel(m_current_task->id);
    } else {
        qWarning() << "audio source error";
        emit error(error_t::mic_source);
        stop_stt();
    }
}

void speech_service::handle_audio_ended() {
    if (audio_source_type() == source_t::file && m_current_task) {
        qDebug() << "file audio source ended successfuly";
    } else {
        qDebug() << "audio source ended successfuly";
    }
}

void speech_service::restart_audio_source(
    const std::variant<QString, stt_source_file_props_t> &config) {
    if (m_stt_engine && m_stt_engine->started()) {
        qDebug() << "creating audio source";

        if (m_source) m_source->disconnect();

        if (std::holds_alternative<stt_source_file_props_t>(config)) {
            m_source = std::make_unique<file_source>(
                std::get<stt_source_file_props_t>(config).file,
                std::get<stt_source_file_props_t>(config).stream_index);
        } else {
            m_source = std::make_unique<mic_source>(std::get<QString>(config));
            if (m_current_task &&
                (m_current_task->flags & task_flags_stt_play_beep)) {
                play_beep(beep_role_t::stt_start_listen);
            }
        }

        set_progress(m_source->progress());
        connect(m_source.get(), &audio_source::audio_available, this,
                &speech_service::handle_audio_available, Qt::QueuedConnection);
        connect(m_source.get(), &audio_source::error, this,
                &speech_service::handle_audio_error, Qt::QueuedConnection);
        connect(m_source.get(), &audio_source::ended, this,
                &speech_service::handle_audio_ended, Qt::QueuedConnection);
    } else if (m_source) {
        m_source.reset();
        set_progress(-1.0);
    }
}

void speech_service::handle_keepalive_timeout() {
    qWarning() << "keepalive timeout => shutting down";
    QCoreApplication::quit();
}

void speech_service::handle_task_timeout() {
    if (m_current_task) {
        qWarning() << "task timeout:" << m_current_task->id;
        if (m_current_task->speech_mode == speech_mode_t::single_sentence)
            stop_keepalive_current_task();
        if (audio_source_type() == source_t::file ||
            audio_source_type() == source_t::mic) {
            cancel(m_current_task->id);
        } else {
            m_current_task.reset();
            emit current_task_changed();
        }
    }
}

void speech_service::update_task_state() {
    refresh_status();

    // 0 = Idle
    // 1 = Speech detected
    // 2 = Processing
    // 3 = Model initialization
    // 4 = Playing Speech
    // 5 = Speech Paused
    // 6 = Canceling

    auto new_task_state = [&] {
        if (m_player.state() == QMediaPlayer::State::PlayingState &&
            m_state == state_t::playing_speech) {
            return 4;
        } else if (m_stt_engine && m_stt_engine->started()) {
            switch (m_stt_engine->speech_detection_status()) {
                case stt_engine::speech_detection_status_t::speech_detected:
                    return 1;
                case stt_engine::speech_detection_status_t::decoding:
                    return 2;
                case stt_engine::speech_detection_status_t::initializing:
                    return 3;
                case stt_engine::speech_detection_status_t::no_speech:
                    break;
            }
        } else if (m_player.state() == QMediaPlayer::State::PausedState ||
                   (m_player.state() == QMediaPlayer::State::StoppedState &&
                    m_state == state_t::playing_speech && m_current_task &&
                    m_current_task->paused)) {
            return 5;
        } else if (m_tts_engine &&
                   m_tts_engine->state() != tts_engine::state_t::idle &&
                   m_tts_engine->state() != tts_engine::state_t::stopped) {
            switch (m_tts_engine->state()) {
                case tts_engine::state_t::speech_encoding:
                case tts_engine::state_t::text_restoring:
                    return 2;
                case tts_engine::state_t::initializing:
                    return 3;
                case tts_engine::state_t::stopping:
                    return 6;
                case tts_engine::state_t::error:
                case tts_engine::state_t::idle:
                case tts_engine::state_t::stopped:
                    break;
            }
        } else if (m_mnt_engine &&
                   m_mnt_engine->state() != mnt_engine::state_t::idle &&
                   m_mnt_engine->state() != mnt_engine::state_t::stopped) {
            switch (m_mnt_engine->state()) {
                case mnt_engine::state_t::translating:
                    return 2;
                case mnt_engine::state_t::initializing:
                    return 3;
                case mnt_engine::state_t::stopping:
                    return 6;
                case mnt_engine::state_t::error:
                case mnt_engine::state_t::idle:
                case mnt_engine::state_t::stopped:
                    break;
            }
        } else if (m_stt_engine && m_stt_engine->stopping()) {
            return 6;
        } else if (m_text_repair_engine &&
                   m_text_repair_engine->state() !=
                       text_repair_engine::state_t::idle &&
                   m_text_repair_engine->state() !=
                       text_repair_engine::state_t::stopped) {
            switch (m_text_repair_engine->state()) {
                case text_repair_engine::state_t::processing:
                    return 2;
                case text_repair_engine::state_t::stopping:
                    return 6;
                case text_repair_engine::state_t::error:
                case text_repair_engine::state_t::idle:
                case text_repair_engine::state_t::stopped:
                    break;
            }
        }

        return 0;
    }();

    if (m_task_state != new_task_state) {
        qDebug() << "task state changed:" << m_task_state << "=>"
                 << new_task_state;
        m_task_state = new_task_state;
        emit task_state_changed();
    }
}

void speech_service::set_state(state_t new_state) {
    if (new_state != m_state) {
        qDebug() << "service state changed:" << m_state << "=>" << new_state;
        m_state = new_state;
        emit state_changed();
    }
}

void speech_service::refresh_status() {
    state_t new_state;

    if (models_manager::instance()->busy() || !feature_discovery_done() ||
        !m_models_changed_handled) {
        new_state = state_t::busy;
    } else if (!models_manager::instance()->has_model_of_role(
                   models_manager::model_role_t::stt) &&
               !models_manager::instance()->has_model_of_role(
                   models_manager::model_role_t::tts) &&
               !models_manager::instance()->has_model_of_role(
                   models_manager::model_role_t::mnt)) {
        new_state = state_t::not_configured;
    } else if (audio_source_type() == source_t::file) {
        new_state = state_t::transcribing_file;
    } else if (audio_source_type() == source_t::mic) {
        if (!m_current_task) {
            qWarning() << "no current task but source is mic";
            new_state = state_t::idle;
        } else if (m_current_task->engine == engine_t::tts) {
            new_state = state_t::playing_speech;
        } else if (m_current_task->engine == engine_t::mnt) {
            new_state = state_t::translating;
        } else if (m_current_task->speech_mode == speech_mode_t::manual) {
            new_state = m_stt_engine && m_stt_engine->started() &&
                                m_stt_engine->speech_status()
                            ? state_t::listening_manual
                            : state_t::idle;
        } else if (m_current_task->speech_mode == speech_mode_t::automatic) {
            new_state = state_t::listening_auto;
        } else if (m_current_task->speech_mode ==
                   speech_mode_t::single_sentence) {
            new_state = state_t::listening_single_sentence;
        } else {
            qWarning() << "unknown speech mode";
            new_state = state_t::unknown;
        }
    } else if (m_current_task && m_current_task->engine == engine_t::tts) {
        if (m_tts_engine &&
            m_tts_engine->state() == tts_engine::state_t::text_restoring) {
            new_state = state_t::repairing_text;
        } else if (m_current_task->speech_mode == speech_mode_t::play_speech) {
            new_state = state_t::playing_speech;
        } else {
            new_state = state_t::writing_speech_to_file;
        }
    } else if (m_current_task && m_current_task->engine == engine_t::mnt) {
        if (m_mnt_engine &&
            m_mnt_engine->state() != mnt_engine::state_t::idle &&
            m_mnt_engine->state() != mnt_engine::state_t::error)
            new_state = state_t::translating;
        else
            new_state = state_t::idle;
    } else if (m_current_task &&
               m_current_task->engine == engine_t::text_repair) {
        if (m_text_repair_engine &&
            m_text_repair_engine->state() ==
                text_repair_engine::state_t::processing) {
            new_state = state_t::repairing_text;
        } else {
            new_state = state_t::idle;
        }
    } else {
        new_state = state_t::idle;
    }

    qDebug() << "service refresh status, new state:" << new_state;

    set_state(new_state);
}

QString speech_service::default_stt_model() const {
    return test_default_stt_model(settings::instance()->default_stt_model());
}

QString speech_service::default_stt_lang() const {
    return m_available_stt_models_map
        .at(test_default_stt_model(settings::instance()->default_stt_model()))
        .lang_id;
}

QString speech_service::default_tts_model() const {
    return test_default_tts_model(settings::instance()->default_tts_model());
}

QString speech_service::default_tts_lang() const {
    return m_available_tts_models_map
        .at(test_default_tts_model(settings::instance()->default_tts_model()))
        .lang_id;
}

QString speech_service::default_ttt_model() const {
    return test_default_ttt_model({});
}

QString speech_service::default_ttt_lang() const {
    return m_available_ttt_models_map.at(test_default_ttt_model({})).lang_id;
}

QString speech_service::default_mnt_lang() const {
    return test_default_mnt_lang(settings::instance()->default_mnt_lang());
}

std::vector<std::reference_wrapper<const speech_service::model_data_t>>
speech_service::model_data_for_lang(
    const QString &lang_id,
    const std::map<QString, model_data_t> &available_models_map) {
    std::vector<std::reference_wrapper<const speech_service::model_data_t>>
        list;

    std::for_each(available_models_map.cbegin(), available_models_map.cend(),
                  [&](const auto &p) {
                      if (p.second.lang_id == lang_id)
                          list.push_back(std::cref(p.second));
                  });

    return list;
}

QString speech_service::default_mnt_out_lang() const {
    return test_default_mnt_out_lang(
        settings::instance()->default_mnt_out_lang());
}

QString speech_service::test_default_model(
    const QString &lang,
    const std::map<QString, model_data_t> &available_models_map) {
    if (available_models_map.empty()) return {};

    auto it = available_models_map.find(lang);

    if (it == available_models_map.cend()) {
        it = std::find_if(
            available_models_map.cbegin(), available_models_map.cend(),
            [&lang](const auto &p) { return p.second.lang_id == lang; });
        if (it != available_models_map.cend()) return it->first;
    } else {
        return it->first;
    }

    return available_models_map.cbegin()->first;
}

QString speech_service::test_default_stt_model(const QString &lang) const {
    return test_default_model(lang, m_available_stt_models_map);
}

QString speech_service::test_default_tts_model(const QString &lang) const {
    return test_default_model(lang, m_available_tts_models_map);
}

QString speech_service::test_default_mnt_model(const QString &lang) const {
    auto it = m_available_mnt_lang_to_model_id_map.find(lang);
    if (it == m_available_mnt_lang_to_model_id_map.end()) return {};
    return it->second;
}

QString speech_service::test_default_mnt_lang(const QString &lang) const {
    auto it = m_available_mnt_lang_to_model_id_map.find(lang);
    if (it == m_available_mnt_lang_to_model_id_map.end()) return {};
    return it->first;
}

QString speech_service::test_mnt_out_lang(const QString &in_lang,
                                          const QString &out_lang) const {
    if (in_lang.isEmpty() || in_lang == out_lang) return {};

    auto models = model_data_for_lang(in_lang, m_available_mnt_models_map);
    if (models.empty()) return {};

    if (out_lang.isEmpty()) return models.front().get().trg_lang_id;

    auto it = std::find_if(models.cbegin(), models.cend(), [&](const auto &wr) {
        return wr.get().trg_lang_id == out_lang;
    });

    if (it != models.cend()) return out_lang;  // direct match

    if (in_lang != QStringLiteral("en") && out_lang != QStringLiteral("en")) {
        // try to find in_lang=>en
        if (std::find_if(models.cbegin(), models.cend(), [&](const auto &wr) {
                return wr.get().trg_lang_id == QStringLiteral("en");
            }) != models.cend()) {  // in_lang=>en exists

            // try to find en=>out_lang
            auto models = model_data_for_lang(QStringLiteral("en"),
                                              m_available_mnt_models_map);

            if (std::find_if(models.cbegin(), models.cend(),
                             [&](const auto &wr) {
                                 return wr.get().trg_lang_id == out_lang;
                             }) != models.cend())
                return out_lang;  // indirect match
        }
    }

    return models.front().get().trg_lang_id;  // fallback
}

QString speech_service::test_default_mnt_out_lang(const QString &lang) const {
    return test_mnt_out_lang(settings::instance()->default_mnt_lang(), lang);
}

QString speech_service::test_default_ttt_model(const QString &lang) const {
    return test_default_model(lang, m_available_ttt_models_map);
}

void speech_service::set_default_stt_model(const QString &model_id) const {
    if (test_default_stt_model(model_id) == model_id) {
        settings::instance()->set_default_stt_model(model_id);
    } else {
        qWarning() << "invalid default stt model";
    }
}

void speech_service::set_default_stt_lang(const QString &lang_id) const {
    settings::instance()->set_default_stt_model(
        test_default_stt_model(lang_id));
}

void speech_service::set_default_mnt_lang(const QString &lang_id) const {
    settings::instance()->set_default_mnt_lang(test_default_mnt_lang(lang_id));
}

void speech_service::set_default_mnt_out_lang(const QString &lang_id) const {
    settings::instance()->set_default_mnt_out_lang(
        test_default_mnt_out_lang(lang_id));
}

void speech_service::set_default_tts_model(const QString &model_id) const {
    if (test_default_tts_model(model_id) == model_id) {
        settings::instance()->set_default_tts_model(model_id);
    } else {
        qWarning() << "invalid default tts model";
    }
}

void speech_service::set_default_tts_lang(const QString &lang_id) const {
    settings::instance()->set_default_tts_model(
        test_default_tts_model(lang_id));
}

QVariantMap speech_service::translations() const {
    QVariantMap map;

    map.insert(QStringLiteral("lang_not_conf"),
               tr("No language has been set."));
    map.insert(QStringLiteral("translation_not_conf"),
               tr("No translator model has been set."));
    map.insert(QStringLiteral("say_smth"), tr("Say something..."));
    map.insert(QStringLiteral("press_say_smth"),
               tr("Press and say something..."));
    map.insert(QStringLiteral("click_say_smth"),
               tr("Click and say something..."));
    map.insert(QStringLiteral("busy_stt"), tr("Busy..."));
    map.insert(QStringLiteral("decoding"), tr("Processing, please wait..."));
    map.insert(QStringLiteral("initializing"),
               tr("Getting ready, please wait..."));
    map.insert(QStringLiteral("translating"), tr("Translating..."));

    return map;
}

speech_service::state_t speech_service::state() const { return m_state; }

int speech_service::current_task_id() const {
    return m_current_task ? m_current_task->id : INVALID_TASK;
}

int speech_service::dbus_state() const { return static_cast<int>(state()); }

void speech_service::start_keepalive_current_task() {
    if (settings::launch_mode != settings::launch_mode_t::service) return;

    m_keepalive_current_task_timer.start();
}

void speech_service::stop_keepalive_current_task() {
    if (settings::launch_mode != settings::launch_mode_t::service) return;

    m_keepalive_current_task_timer.stop();
}

void speech_service::remove_cached_files() {
    if (settings::instance()->cache_policy() ==
        settings::cache_policy_t::CacheRemove) {
        QDir dir{settings::instance()->cache_dir()};

        // media files

        dir.setNameFilters(QStringList{} << "*.wav"
                                         << "*.mp3"
                                         << "*.ogg"
                                         << "*.opus"
                                         << "*.flac");
        dir.setFilter(QDir::Files);

        for (const auto &file : std::as_const(dir).entryList())
            dir.remove(file);

        // openvino chache

        dir.setNameFilters(QStringList{} << "*-encoder-openvino-cache");
        dir.setFilter(QDir::Dirs);

        for (const auto &file : std::as_const(dir).entryList())
            QDir{dir.absoluteFilePath(file)}.removeRecursively();
    }
}

static void add_to_env_path(const QString &dir) {
    try {
        auto *old_path = getenv("PATH");
        if (old_path)
            setenv("PATH",
                   fmt::format("{}:{}", dir.toStdString(), old_path).c_str(),
                   true);
        else
            setenv("PATH", dir.toStdString().c_str(), false);
    } catch (const std::runtime_error &err) {
        qWarning() << "error:" << err.what();
    }
}

void speech_service::setup_env() {
    auto mbrola_bin_dir = module_tools::path_to_bin_dir_for_path("mbrola");
    qDebug() << "mbrola dir:" << mbrola_bin_dir;
    if (!mbrola_bin_dir.isEmpty()) add_to_env_path(mbrola_bin_dir);

    auto espeak_bin_dir = module_tools::path_to_bin_dir_for_path("espeak");
    qDebug() << "espeak dir:" << espeak_bin_dir;
    if (!espeak_bin_dir.isEmpty() && espeak_bin_dir != mbrola_bin_dir)
        add_to_env_path(espeak_bin_dir);

    if (auto ver = settings::instance()->gpu_overrided_version();
        settings::instance()->gpu_override_version() && !ver.isEmpty()) {
        qDebug() << "overrided gpu version:" << ver;
        gpu_tools::rocm_override_gfx_version(ver.toStdString());
    }

    const auto *has_overide_ver = getenv("HSA_OVERRIDE_GFX_VERSION");
    if (has_overide_ver)
        qDebug() << "HSA_OVERRIDE_GFX_VERSION:" << has_overide_ver;
}

void speech_service::setup_modules() {
    module_tools::init_module(QStringLiteral("rhvoicedata"));
    module_tools::init_module(QStringLiteral("rhvoiceconfig"));
    module_tools::init_module(QStringLiteral("espeakdata"));
}

// DBus

int speech_service::SttStartListen(int mode, const QString &lang,
                                   const QString &out_lang) {
    qDebug() << "[dbus => service] called StartListen:" << lang << mode
             << out_lang;
    m_keepalive_timer.start();

    speech_mode_t speech_mode;

    if (mode == 0)
        speech_mode = speech_mode_t::automatic;
    else if (mode == 1)
        speech_mode = speech_mode_t::manual;
    else if (mode == 2)
        speech_mode = speech_mode_t::single_sentence;
    else {
        qWarning() << "invalid speech mode";
        return INVALID_TASK;
    }

    return stt_start_listen(speech_mode, lang, out_lang, {});
}

int speech_service::SttStartListen2(int mode, const QString &lang,
                                    const QString &out_lang,
                                    const QVariantMap &options) {
    qDebug() << "[dbus => service] called StartListen2:" << lang << mode
             << out_lang;
    m_keepalive_timer.start();

    speech_mode_t speech_mode;

    if (mode == 0)
        speech_mode = speech_mode_t::automatic;
    else if (mode == 1)
        speech_mode = speech_mode_t::manual;
    else if (mode == 2)
        speech_mode = speech_mode_t::single_sentence;
    else {
        qWarning() << "invalid speech mode";
        return INVALID_TASK;
    }

    return stt_start_listen(speech_mode, lang, out_lang, options);
}

int speech_service::SttStopListen(int task) {
    qDebug() << "[dbus => service] called StopListen:" << task;
    m_keepalive_timer.start();

    return stt_stop_listen(task);
}

int speech_service::Cancel(int task) {
    qDebug() << "[dbus => service] called Cancel:" << task;
    m_keepalive_timer.start();

    return cancel(task);
}

int speech_service::SttTranscribeFile(const QString &file, const QString &lang,
                                      const QString &out_lang,
                                      const QVariantMap &options) {
    qDebug() << "[dbus => service] called TranscribeFile:" << file << lang
             << out_lang;
    start_keepalive_current_task();

    return stt_transcribe_file(file, lang, out_lang, options);
}

double speech_service::SttGetFileTranscribeProgress(int task) {
    qDebug() << "[dbus => service] called GetFileTranscribeProgress:" << task;
    start_keepalive_current_task();

    return stt_transcribe_file_progress(task);
}

int speech_service::KeepAliveService() {
    qDebug() << "[dbus => service] called KeepAliveService";
    m_keepalive_timer.start();

    return m_keepalive_timer.remainingTime();
}

int speech_service::KeepAliveTask(int task) {
    qDebug() << "[dbus => service] called KeepAliveTask:" << task;
    m_keepalive_timer.start();

    if (m_current_task && m_current_task->id == task) {
        m_keepalive_current_task_timer.start();
        return m_keepalive_current_task_timer.remainingTime();
    }

    qWarning() << "invalid task:" << task;

    return 0;
}

int speech_service::TtsPlaySpeech(const QString &text, const QString &lang) {
    qDebug() << "[dbus => service] called TtsPlaySpeech:" << lang;
    start_keepalive_current_task();

    return tts_play_speech(text, lang, QVariantMap{});
}

int speech_service::TtsPlaySpeech2(const QString &text, const QString &lang,
                                   const QVariantMap &options) {
    qDebug() << "[dbus => service] called TtsPlaySpeech2:" << lang;
    start_keepalive_current_task();

    return tts_play_speech(text, lang, options);
}

int speech_service::TtsSpeechToFile(const QString &text, const QString &lang,
                                    const QVariantMap &options) {
    qDebug() << "[dbus => service] called TtsSpeechToFile:" << lang;
    start_keepalive_current_task();

    return tts_speech_to_file(text, lang, options);
}

double speech_service::TtsGetSpeechToFileProgress(int task) {
    qDebug() << "[dbus => service] called TtsGetSpeechToFileProgress:" << task;
    start_keepalive_current_task();

    return tts_speech_to_file_progress(task);
}

int speech_service::TtsPauseSpeech(int task) {
    qDebug() << "[dbus => service] called TtsPauseSpeech";
    start_keepalive_current_task();

    return tts_pause_speech(task);
}

int speech_service::TtsResumeSpeech(int task) {
    qDebug() << "[dbus => service] called TtsResumeSpeech";
    start_keepalive_current_task();

    return tts_resume_speech(task);
}

int speech_service::TtsStopSpeech(int task) {
    qDebug() << "[dbus => service] called TtsStopSpeech";
    start_keepalive_current_task();

    return tts_stop_speech(task);
}

int speech_service::MntTranslate(const QString &text, const QString &lang,
                                 const QString &out_lang) {
    qDebug() << "[dbus => service] called MntTranslate";
    start_keepalive_current_task();

    return mnt_translate(text, lang, out_lang, {});
}

int speech_service::MntTranslate2(const QString &text, const QString &lang,
                                  const QString &out_lang,
                                  const QVariantMap &options) {
    qDebug() << "[dbus => service] called MntTranslate2";
    start_keepalive_current_task();

    return mnt_translate(text, lang, out_lang, options);
}

QVariantMap speech_service::MntGetOutLangs(const QString &lang) {
    qDebug() << "[dbus => service] called MntGetOutLangs";
    return mnt_out_langs(lang);
}

int speech_service::TttRepairText(const QString &text,
                                  const QVariantMap &options) {
    qDebug() << "[dbus => service] called TttRepairText";
    start_keepalive_current_task();

    return ttt_repair_text(text, options);
}

QVariantMap speech_service::FeaturesAvailability() {
    qDebug() << "[dbus => "
                "service] "
                "called "
                "FeaturesAvailab"
                "ility";
    return features_availability();
}

int speech_service::Reload() {
    qDebug() << "[dbus => service] called Reload";
    m_keepalive_timer.start();

    m_models_changed_handled = false;

    models_manager::instance()->reload();

    refresh_status();

    return SUCCESS;
}
