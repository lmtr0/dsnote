/* Copyright (C) 2023-2025 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import QtQuick.Controls 2.15

DialogPage {
    id: root

    title: qsTr("Changes")

    SectionLabel {
        text: qsTr("Version %1").arg(APP_VERSION)
    }

    RichLabel {
        text: "<p>" + qsTr("General") + "</p>
        <ul>
        <li>Fix: The model cannot be activated when the license file cannot be downloaded due to an error.</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Fix: App crashes when WhisperCpp is used on certain CPUs.</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>Fix: The Coqui XTTS model license cannot be downloaded.</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>Fix: App crashes when the translator is used on certain CPUs.</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.8.2")
    }

    RichLabel {
        text: "<p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>New <i>Piper</i> voices for Argentine Spanish, Hindi, Malayalam and Nepali</li>
        <li>Fix: Using the <i>Coqui TTS</i> engine causes the app to crash on some platforms.</li>
        <li>Fix: Crash when the TTS engine generates a corrupted audio file</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>New languages enabled in Whisper: Azerbaijani, Belarusian, Kannada, Malayalam, Tamil</li>
        </ul>
        <p>Flatpak</p>
        <ul>
        <li>Downgrade <i>numba</i> Python package to 0.60.0 version</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.8.1")
    }

    RichLabel {
        text: "<p>" + qsTr("Translator") + "</p>
        <ul>
        <li>Fix: Model download error for Portuguese, Dutch, Persian, Norwegian and Icelandic languages.</li>
        <li>Updated models with improved accuracy: German to English, Dutch to English, English to Ukrainian,
            English to Hungarian, English to Catalan, Catalan to English, English to Lithuanian, English to Latvian,
            English to Slovenian, Slovenian to English, English to Slovak, English to Russian.</li>
        <li>New models: Azerbaijani to English, Belarusian to English, Bengali to English, Gujarati to English,
            Hebrew to English, Hindi to English, Kannada to English, Malayalam to English, Malay to English,
            Albanian to English, Tamil to English.</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>New very large <i>Vosk</i> model for German langauge: <i>Tuda-DE Large</i></li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>Coqui MMS models for the following new languages: Kannada, Malayalam, Tamil</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li><i>Speech Note</i> has been translated into German language.</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.8.0")
    }

    RichLabel {
        text: "<p>" + qsTr("General") + "</p>
        <ul>
        <li>Case-sensitive matching in <i>Rules</i>.
            To enable case-sensitive matching in text, use the option <i>Case sensitive</i> in the rule editing form.</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li><i>Speech Note</i> has been translated into Arabic, Chinese, Catalan, Spanish, Turkish and French-Canadian languages.</li>
        <li>Command line option and DBus API for exporting synthesized speech to an audio file instead of playing it aloud.
            Use <i>--output-file</i> together with <i>start-reading-clipboard</i> and <i>start-reading-text</i> actions.</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>New <i><a href='https://huggingface.co/nyrahealth/CrisperWhisper'>CrisperWhisper</a></i> model for <i>FasterWhisper</i> engine.
            <i>CrisperWhisper</i> is designed for fast, precise, and verbatim speech recognition with accurate word-level timestamps.
            Unlike the original Whisper, which tends to omit disfluencies and follows more of a intended transcription style,
            <i>CrisperWhisper</i> aims to transcribe every spoken word exactly as it is, including fillers, pauses, stutters and false starts.
            <i>CrisperWhisper</i> model is enabled only for English and German languages.</li>
        <li><i><a href='https://huggingface.co/KBLab'>KBLab Whisper</a></i> models for Swedish.
            The National Library of Sweden has released fine-tuned STT models trained on its library collections.
            The models have significantly improved accuracy compared to regular Whisper models.
            Even \"Tiny\" model provides decent quality.
            KBLab models are available as both <i>FasterWhisper</i> and <i>WhisperCpp</i>.</li>
        <li><i><a href='https://keyboard.futo.org/voice-input-models'>FUTO<a/></i> Whisper models.
            New models used in the FUTO mobile keyboard app.</li>
        <li>Using an existing note as the initial context in decoding.
            This has the potential to improve transcription quality and reduce \"hallucination\" problem.
            If you observe a degradation in quality, turn off the <i>Use note as context</i> option.</li>
        <li>Option to pause listening while processing.
            This option can be useful when <i>Listening mode</i> is <i>Always on</i>.
            By default, listening continues even when a piece of audio data is being processed.
            Using this option, you can temporarily pause listening for the duration of processing.</li>
        <li>Option to play an audible tone when starting and stopping listening</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li><i><a href='https://github.com/hexgrad/kokoro'>Kokoro</a></i> TTS engine.
            Kokoro is a compact yet powerful open-source multilingual TTS engine.
            Despite its modest size (trained on less than 100 hours of audio), it delivers impressive results.
            Unlike other engines with comparable quality, Kokoro offers the advantage of running even on devices without GPUs.
            Kokoro voices are enabled for: English, Chinese, Japanese, Hindi, Italian, French, Spanish and Portuguese.</li>
        <li><i><a href='https://github.com/SWivid/F5-TTS'>F5-TTS</a></i> engine.
            The F5-TTS provides exceptional voice cloning capabilities. The currently enabled model works with English and Chinese languages.
            To work well, F5-TTS needs audio samples of the voice along with the transcribed text spoken in the sample.
            Make sure that the text spoken in the audio sample is set correctly in <i>Voice profiles</i> &rarr; <i>Audio sample</i> menu.
            F5-TTS works best with CUDA acceleration. CPU only processing can be very slow.</li>
        <li><i><a href='https://github.com/huggingface/parler-tts'>Parler-TTS</a></i> engine. Parler-TTS can generate high-quality, natural sounding speech in the style of a given speaker (gender, pitch, speaking style, etc).
            The speaker's characteristics are defined by a text description (prompt).
            To use Parler-TTS models, you need to configure a <i>Text voice profile</i>. This can be done in the <i>Voice profiles</i> menu.
            To learn more about the unique features of this engine, check out <a href='https://github.com/huggingface/parler-tts/'>Parler TTS project website</a>.
            Parler-TTS primarily supports English, but a multilingual model for French, Spanish, Portuguese, Polish, German, Dutch and Italian is also included.
            Currently, the multilingual model provides rather poor quality and not entirely usable speech.
            Parler-TTS works best with CUDA acceleration. CPU only processing can be very slow.</li></li>
        <li><i><a href='https://github.com/s-macke/SAM'>S.A.M.</a></i> TTS engine. S.A.M. is a small speech synthesizer designed for the Commodore 64.
            It features a robotic voice that evokes a strong sense of nostalgia.
            The S.A.M. voice is available in English only.</li>
        <li><i>Normalize audio</i> setting option.
            Use this option to enable/disable audio volume normalization.
            The volume is normalized independently for each sentence, which can lead to unstable volume levels in different sentences.
            Disable this option if you observe this problem.</li>
        <li>New <i>Piper</i> voices for Dutch, Finnish, German and Luxembourgish</li>
        <li>New <i>RHVoice</i> voice for Spanish</li>
        <li>Updated <i>RHVoice</i> voice for Czech</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New models: English to Chinese, English to Arabic, Arabic to English, English to Korean, English to Japanese</li>
        </ul>
        <p>" + qsTr("Accessibility") + "</p>
        <ul>
        <li>Support for <i>Insert into active window</i> under Wayland.
            Using <i>start-listening-active-window</i> or <i>start-listening-translate-active-window</i>
            actions you can directly insert the decoded text into any window which is currently in focus.
            This feature worked under X11 only but now it is also suppored under Wayland.
            For actions to work, <i>ydotool</i> daemon must be installed and running.
            If you are using Flatpak, also make sure that the application has permission to access <i>ydotool</i> daemon socket file.</li>
        <li>Support for <i>Global keyboard shortcuts</i> under Wayland.
            Global keyboard shortcuts allow you to start or stop listening and reading using keyboard
            even when the application is not active (e.g. minimized or in the background).
            Until now, this capability was only available under X11.
            Now integration with <i>XDG Desktop Portal</i> has been added, making global keyboard shortcuts possible also under Wayland.
            For shortcuts to work, your desktop enviroment has to support
            <i>GlobalShortcuts</i> interface on <i>XDG Desktop Portal</i> service.</li>
        </ul>
        <p>Flatpak</p>
        <ul>
        <li>Python support enabled in <i>Tiny</i> and ARM packages.
            Python libraries are not included in Tiny or ARM packages, but using the <i>Location of Python libraries</i> option,
            you can set an external directory that contains the libraries.
            Make sure that the Flatpak application has permissions to access this directory.</li>
        <li>Flatpak runtime update to version 5.15-24.08</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.7.1")
    }

    RichLabel {
        text: "<p>" + qsTr("General") + "</p>
        <ul>
        <li>Fix: The application failed to start when the processor did not support AVX CPU extension.</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New models: Korean to English, Japanese to English</li>
        <li>Updated models: Chinese to English</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.7.0")
    }

    RichLabel {
        text: "<p>" + qsTr("General") + "</p>
        <ul>
        <li><i>Rules</i> for text transformations that can be applied after Speech to Text or before Text to Speech.
            With <i>Rules</i>, you can easily and flexibly correct errors in decoded text or correct mispronounced words.
            <i>Rules</i> are disabled by default. To use them, you must enable them in the settings (<i>General</i> &rarr; <i>Show Rules</i>).</li>
        <li>New modes for inserting text at the cursor position or replacing the current note.
            To insert text at the cursor position rather than at the end of the note,
            change <i>Text appending mode</i> option to <i>Add at the cursor position</i> in the settings.
            When the <i>Replace an existing note</i> option is set, whenever new text is added, it will replace the existing note.</li>
        <li>DBus API for integration with external applications</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li><i>Speech Note</i> has been translated into Slovenian language.</li>
        <li>Status indication in the system tray icon.
            When using the system tray icon, statuses such as processing, listening, etc. are presented with an animated tray icon.</li>
        <li>Models grouped by type in model browser.
            To improve usability, instead of a list containing models of all types, models are grouped by type in separate tabs.</li>
        <li>New <i>General</i> and <i>Advanced</i> tabs in <i>Settings</i>.</li>
        <li>Command line options for printing available or active model.
            Use <i>--print-available-models</i> or <i>--print-active-model</i> to list all available models or the currently active model.</li>
        <li>Command line option to print the current status of the application.
            Use <i>--print-state</i> to see the current state.
            This option can be useful when integrating with external programs or widgets on the desktop.
            To see all available command line options, run the command: <i>" + (_settings.is_flatpak() ? "flatpak run net.mkiol.SpeechNote" : "dsnote") + " --help</i></li>
        </li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Support for <i>Vulkan</i> GPU acceleration in <i>WhisperCpp</i>.
            Vulkan acceleration enables much faster STT decoding with Intel, AMD or NVIDIA graphics cards.
            With Vulkan, decoding is quicker than with OpenVINO, OpenCL and ROCm, but still may be slightly slower compared to CUDA.
            The biggest advantage of Vulkan is that you can use it without installing any GPU acceleration add-ons.
            As a reminder, GPU acceleration is not enabled by default. Use the option in <i>Settings</i> to turn it on.</li>
        <li>New <i>Whisper Large Turbo</i> model for both <i>WhisperCpp</i> and <i>FasterWhisper</i>.
            <i>Turbo</i> is a finetuned version of a pruned <i>Whisper Large-v3</i>. It's the exact same model,
            except that the number of decoding layers have reduced. As a result, the model is way faster,
            at the expense of a minor quality degradation.
            <i>Turbo</i> model does not have the ability to translate into English, as does the regular Large model.</li>
        <li>Simplified engine configuration options. Instead of multiple options, you can now select a Profile,
            which allows you to change the engine's processing parameters.
            There are three profiles to choose from: <i>Best Performance</i>, <i>Best Quality</i> and <i>Custom</i>.</li>
        <li>Echo mode. After processing, the decoded text will be immediately read out using the currently set Text to Speech model.
            To enable, use the option in the settings (<i>Speech to Text</i> &rarr; <i>Echo mode</i>).</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>New <i>Piper</i> voice for Latvian</li>
        <li>New <i>WhisperSpeech Small</i> model for: English, Italian, German, French, Spanish, Dutch and Portuguese</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New models: English to Finnish, English to Turkish, English to Swedish, Swedish to English,
            English to Slovak, English to Indonesian, English to Romanian, English to Greek, Chinese to English</li>
        <li>Updated models: English to Catalan, English to Russian, English to Ukrainian, English to Czech</li>
        </ul>
        <p>" + qsTr("Accessibility") + "</p>
        <ul>
        <li>Option to scan special key strokes when setting keyboard shortcuts (X11 only).
            If you want to use special keys as shortcuts (so-called \"multimedia keys\"),
            instead of typing their names, you can automatically set the key by pressing it.</li>
        <li>Keyboard shortcuts enabled for several user interface elements.
            Elements such as menu items or buttons can be controlled using the keyboard shortcuts.
            Examples: Switch to Notepad (Ctrl+N), Switch to Translator (Ctrl+T), Open Languages (Ctrl+L),
            Read (Ctrl+Alt+Shift+R), Listen (Ctrl+Alt+Shift+L), Stop (Ctrl+Alt+Shift+S),
            Cancel (Ctrl+Alt+Shift+C), Pause (Ctrl+Alt+Shift+P) and more...</li>
        <li>New <i>Actions</i> and global keyboard shortcut to force translation of text in STT:
            <i>start-listening-translate</i>, <i>start-listening-translate-active-window</i>, <i>start-listening-translate-clipboard</i>.
            The decoded text is always translated into English when the \"translate\" action is triggered.
            This only works when using Whisper models.</li>
        <li>New <i>Action</i> to read text from the command-line option: <i>start-reading-text</i>.
            To pass text, use the <i>--text</i> option in the command-line interface.</li>
        <li>Fix: <i>Insert into active window</i> didn't work for all applications.</li>
        </ul>
        <p>Flatpak</p>
        <ul>
        <li>whisper.cpp update to version 1.7.1</li>
        <li>PyTorch update to version 2.5.1</li>
        <li>The Flatpak GPU acceleration add-on for AMD is no longer recommended.
            If you have already installed the AMD add-on, you should uninstall it.
            Better results can be achieved with Vulkan acceleration, which does not require the add-on.</li>
        </ul>
        <p>Video presentation of all new features: <a href='https://www.youtube.com/watch?v=cEht4Fts6Bo'>https://www.youtube.com/watch?v=cEht4Fts6Bo</a></p>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.6.1")
    }

    RichLabel {
        text: "<p>General:</p>
        <ul>
        <li>Fix: The application failed to start when the processor did not support the required CPU extension.</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Swedish translation has been updated.</li>
        </ul>
        <p>" + qsTr("Accessibility") + "</p>
        <ul>
        <li>Fix: Special keyboard keys were not supported as a keyboard shortcut.
            Examples: <i>Favorites</i>, <i>Launch Mail</i>, <i>Refresh</i>, <i>Home Page</i>, <i>Calculator</i> and many more...</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New models: English to Latvian, English to Danish, English to Croatian, English to Slovenian,
            Indonesian to English, Romanian to English</li>
        <li>Updated models: English to Hungarian, Czech to English, Greek to English</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.6.0")
    }

    RichLabel {
        text: "<p>" + qsTr("User Interface") + "</p>
        <ul>
        <li><i>Speech Note</i> has been translated into Norwegian language.</li>
        <li>Grouped models.
            Models that provide multiple sub-models (for example, TTS models that provide different voices)
            are shown in groups. This makes it easier to find models in the model browser.</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>The name of the all Whisper models has been changed to <i>WhisperCpp</i> to better reflect the engine behind them.
            Whisper is currently supported by the <i>WhisperCpp</i> and <i>FasterWhisper</i> engines.
            Both engines are optimized to achieve the best performance.</li>
        <li>Automatic language detection in STT.
            To automatically detect the language during STT, select one of the models that is in the <i>Auto detected</i>
            category in the language list. STT with <i>Auto detected</i> models are slower than models with a defined language,
            so if you know the language, it is recommended to use models for a specific language.</li>
        <li>Separate settings for engines.
            The configuration of each engine has been separated in the settings.
            You can separately set the parameters for <i>WhisperCpp</i> and <i>FasterWhisper</i>.
            The new configuration parameters that have been added to the settings are:
            <i>Number of simultaneous threads</i>, <i>Beam search width</i>,
            <i>Audio context size</i>, <i>Use Flash Attention</i>.
            The default settings were chosen to get the shortest STT decoding time, which may in some cases result in slightly worse accuracy.
            If you notice a degradation in accuracy, change the decoding parameters.</li>
        <li>Quicker decoding with <i>WhisperCpp</i>.
            Optimization for short sentences has been added to <i>WhisperCpp</i> engine. With it, the speed of STT has doubled.</li>
        <li>Support for <i>OpenVINO</i> hardware acceleration in <i>WhisperCpp</i>.
            <i>OpenVINO</i> hardware acceleration has been enabled for <i>WhisperCpp</i> engine and selected models.
            With <i>OpenVINO</i> decoding on CPU is much quicker. If you are not using GPU acceleration,
            it is recommended to enable <i>OpenVINO</i> in <i>WhisperCpp</i> engine settings.
            Currently, <i>OpenVINO</i> is enabled only for CPU acceleration.</li>
        <li>Option for inserting processing statistics.
            New settings option allows inserting processing related information to the text after decoding,
            such as processing time and audio length. This can be useful for comparing the performance of different models,
            engines and their parameters. Option works only with <i>WhisperCpp</i> and <i>FasterWhisper</i> engines
            (<i>Speech to Text</i> &rarr; <i>Insert statistics</i>).</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>Control tags for advance TTS processing.
            Control tags allow you to dynamically change the speed of synthesized text or add silence between sentences.
            To use control tags, insert <i>{speed: 0.5}</i> or <i>{silence: 1s}</i> into the text.
            To learn more about control tags go to <i>Text to Speech</i> &rarr; <i>Use control tags</i> in the settings.</i>
            For convenience, you can also insert predefined control tags using text context menu <i>Insert control tag</i>.</li>
        <li>Welsh language. New language is enabled with <i>Piper</i> voice.</li>
        <li>New <i>Piper</i> voices for Spanish, Italian and English</li>
        <li>New <i>RHVoice</i> voices for Slovak and Croatian</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>Improved Translator UI. The <i>Translate</i>, <i>Switch languages</i> and <i>Add</i> buttons have been
            placed between text areas which is more convenient.</li>
        <li>Support for older hardware.
            Until now, the translator did not work on older processors without CPU AVX extension. Now there is no such restriction anymore.</li>
        <li>New models: English to Lithuanian, Croatian to English, Latvian to English, Danish to English, Serbian to English,
            Slovak to English, Bosnian to English, Vietnamese to English</li>
        <li>Updated models: Lithuanian to English, Slovenian to English, Russian to English, Ukrainian to English</li>
        </ul>
        <p>Flatpak:</p>
        <ul>
        <li>New library: OpenVINO version 2024.1.0.15008</li>
        <li>whisper.cpp update to version 1.6.2</li>
        <li>CTranslate2 update to version 4.3.1</li>
        </ul>
        <p>Video presentation of all new features: <a href='https://www.youtube.com/watch?v=AVW5OY63wjg'>https://www.youtube.com/watch?v=AVW5OY63wjg</a></p>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.5.0")
    }

    RichLabel {
        text: "<p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Import subtitles embedded into video file.
            If your video file contains one or many subtitle streams, you can import the selected subtitles into notepad.</li>
        <li>Support for more subtitles formats. You can import and export subtitles in SRT, WebVTT and ASS formats.</li>
        <li>Unified file importing and exporting.
            Text, subtitles, audio and video files can be imported or exported using unified menu bar option
            (<i>Import from a file</i> or <i>Export to a file</i>).</li>
        <li>Settings option to enable/disable remembering the last note.
            If the option is disabled, the last note will not be available after restarting the app.
            (<i>User Interface</i> &rarr; <i>Remember the last note</i>)</li>
        <li>Settings option for default action when importing note from a file (<i>User Interface</i> &rarr; <i>File import action</i>).
            You can set <i>Ask whether to add or replace</i>, <i>Add to an existing note</i> or <i>Replace an existing note</i>.</li>
        <li>Enhanced text editor font settings. You can set the font family, style and size of the font used in the text editor
            (<i>User Interface</i> &rarr; <i>Font in text editor</i>).</li>
        <li>Text to Text repair options.
            With these options you can directly fix diacritical marks and punctuation in the text.
            Options are hidden by default. Use <i>User Interface</i> &rarr; <i>Show Repair text options</i> to enable them.
            To use <i>Restore punctuation</i> and <i>Restore diacritical marks</i> you need to download additional models.</li>
        <li>Text context menu with additional options: <i>Read selection</i> and <i>Translate selection</i>.
            To activate context menu use mouse right click.</li>
        <li>New text appending style: <i>After empty line</i></li>
        <li>System tray menu for changing active STT/TTS model</li>
        <li>User friendly names of audio input devices (<i>Speech to Text</i> &rarr; <i>Audio input device</i>)</li>
        <li>Simplified model filtering. It is now less flexible, but much easier to understand and use.</li>
        <li><i>Speech Note</i> has been translated into Ukrainian and Russian languages.</li>
        <li>Fix: Cancellation was blocking the user interface.</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Updated <a href='https://huggingface.co/distil-whisper/distil-large-v3'>Distil</a> model for English: <i>Distil Large-v3</i>.
            New model is enabled for WhisperCpp and FasterWhisper engines.</li>
        <li>New Fine-Tuned WhisperCpp and FasterWhisper models for
            <a href='https://huggingface.co/samolego/whisper-small-slovenian'>Slovenian</a> and
            <a href='https://huggingface.co/Aspik101/distil-whisper-large-v3-pl'>Polish</a></li>
        <li>Fix: Punctuation model could not be downloaded.</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li><a href='https://collabora.github.io/WhisperSpeech/'>WhisperSpeech</a> engine
            that generates voice with exceptional naturalness.
            The new engine comes with models for English and Polish languages. All models support voice cloning.</li>
        <li>New voice cloning model for Vietnamese: <a href='https://huggingface.co/capleaf/viXTTS'>viXTTS</a>.
            Model is a fine-tuned version of the phenomenal <i>Coqui XTTS</i>.</li>
        <li>New Piper voices for English, Persian, Slovenian, Turkish, French and Spanish</li>
        <li>New RHVoice voice for Czech</li>
        <li>Settings option to enable/disable speech synchronization with subtitle timestamps
            (<i>Text to Speech</i> &rarr; <i>Sync speech with timestamps</i>).
            This may be useful for creating voice overs.</li>
        <li>Mixing speech with audio from an existing file.
            When exporting to a file, you can overlay speech with audio from an existing media file.
            This can be useful when creating voice overs from subtitles
            (<i>Export to a file</i> &rarr; <i>Mix speech with audio from an existing file</i>).</li>
        <li>Context menu option to read from cursor position or read only selected text.
            To activate context menu use mouse right click.</li>
        <li>Speech audio is always normalized after TTS processing.</li>
        <li>Fix: Mimic3 models could not be downloaded.</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New models: Greek to English, Maltese to English,
            Slovenian to English, Turkish to English, English to Catalan</li>
        <li>Updated models: Czech and Lithuanian</li>
        <li>Handy buttons to quickly add translated text to the note or
            to replace it and switch languages (<i>Add</i>, <i>Replace</i>).</li>
        <li>Context menu option to translate from cursor position or translate only selected text.
            To activate context menu use mouse right click.</li>
        </ul>
        <p>" + qsTr("Accessibility") + "</p>
        <ul>
        <li>New <i>Actions</i> for STT/TTS models switching: <i>switch-to-next-stt-model</i>, <i>switch-to-prev-stt-model</i>,
            <i>switch-to-next-tts-model</i>, <i>switch-to-prev-tts-model</i>, <i>set-stt-model</i>, <i>set-tts-model</i></li>
        <li>New global keyboard shortcuts for STT/TTS models switching (X11 only): <i>Switch to next STT model</i>,
            <i>Switch to prev STT model</i>, <i>Switch to next TTS model</i>, <i>Switch to prev TTS model</i></li>
        <li>Toggle option for keyboard shortcuts (X11 only). When <i>Toggle behavior</i> is enabled,
            Start listening/reading shortcuts will also stop listening/reading if they are triggered while listening/reading is active.</li>
        <li>Fix: Accented characters (e.g.: ã, ê) were not transferred correctly to the active window.</li>
        </ul>
        <p>Flatpak:</p>
        <ul>
        <li>Flatpak runtime update to version 5.15-23.08</li>
        <li>AMD ROCm update to version 5.7.3</li>
        <li>PyTorch update to version 2.2.1</li>
        <li>CTranslate2 update to version 4.2.1</li>
        <li>Faster-Whisper update to version 1.0.2</li>
        </ul>
        <p>Video presentation of all new features: <a href='https://www.youtube.com/watch?v=S9MJ7y8-bcw'>https://www.youtube.com/watch?v=S9MJ7y8-bcw</a></p>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.4.0")
    }

    RichLabel {
        text: "<p>Flatpak:</p>
        <ul>
        <li>Modular Flatpak package.
            The application package is divided into a base package <i>Speech Note</i> (<i>net.mkiol.SpeechNote</i>) and
            two optional add-ons: <i>Speech Note AMD</i> (<i>net.mkiol.SpeechNote.amd</i>) and <i>Speech Note NVIDIA</i> (<i>net.mkiol.SpeechNote.nvidia</i>).
            Add-ons packages provide a set of libraries for GPU acceleration with AMD and NVIDIA graphics cards.
            New \"modular\" approach makes the base Flatpak package much smaller.</li>
        <li>NVIDIA CUDA runtime update to version 12.2</li>
        <li>AMD ROCm runtime update to version 5.6</li>
        <li>PyTorch update to version 2.1.1</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Improvements to the model browser.
            You can check various model properties such as size, license, and the URLs from which the model is downloaded.</li>
        <li>Model filtering options.
            Models can be searched by various features such as: Engine type, Processing speed, Quality, Additional capabilities.</li>
        <li>Setting option to minimize to the system tray.
            To enable system tray icon, use <i>User Interface</i> &rarr; <i>Use system tray icon</i> option.
            To always start the application minimized to the system tray, use <i>User Interface</i> &rarr; <i>Start minimized to the system tray</i> option.
            To start the application minimized only once, use <i>--start-in-tray</i> command line option.</li>
        <li>Setting option to enable/disable including of recognized or read text in desktop notifications
            (<i>User Interface</i> &rarr; <i>Include recognized or read text in notifications</i>)</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Marathi language. New language is enabled with <i>WhisperCpp</i> and <i>FasterWhisper</i> models.</li>
        <li>New version of FasterWhisper Large model: <i>FasterWhisper Large-v3</i></li>
        <li>New <i>Distil</i> Faster Whisper models for English.
            Distil models are potentially faster than regular models.</li>
        <li>WhisperCpp and FasterWhisper enabled for Chinese-Cantonese language (广东话)</li>
        <li>Support for Speex audio codec in <i>Transcribe a file</i></li>
        <li>Translate to English option for WhisperCpp and FasterWhisper models.
            To automatically translate to English, use the switch to the right of the model selection box.
            The option is only visible if you select a non-English <i>WhisperCpp</i> or <i>FasterWhisper</i> model.</li>
        <li>More effective GPU acceleration for Whisper models with AMD graphics cards.
            Average decoding time has been shortened by 3 times.
            If you are using Flatpak package, install <i>net.mkiol.SpeechNote.amd</i> add-on
            to enable full support for AMD ROCm acceleration.
            GPU acceleration for AMD graphics cards works only with <i>WhisperCpp</i> models.
            It does not work with <i>FasterWhisper</i> models.</li>
        <li>Subtitles generation. To generate timestamped text in SRT format,
            change the text format using the drop-down menu at the bottom of the text area.
            Check the settings to find the subtitle options.</li>
        <li>Support for multiple audio streams in a video file. If the video file contains more than one audio stream,
            you are asked to select which one to transcribe.</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>Marathi language. New language is enabled with Coqui MMS model.</li>
        <li>Voice cloning with Coqui XTTS and YourTTS models.
            Coqui XTTS models are enabled for: Arabic, Brazilian Portuguese, Chinese, Czech,
            Dutch, English, French, German, Hungarian, Italian, Japanese, Korean, Polish, Russian,
            Spanish and Turkish.
            Coqui YourTTS model is enabled for: English, French and Brazilian Portuguese.
            Text to Speech with Coqui XTTS models might be slow when GPU acceleration is not enabled.</li>
        <li>Voice samples creator.
            A reference voice sample is used for voice cloning.
            You can create voice sample with a microphone or from audio or video file.
            The sample creator is available on main toolbar (<i>Voice samples</i>)
            only if the selected TTS model supports voice cloning.</li>
        <li>New voices for Serbian and Uzbek languages (RHVoice models)</li>
        <li>GPU acceleration for Coqui models with AMD graphics cards.
            If you are using Flatpak package, install <i>net.mkiol.SpeechNote.amd</i> add-on
            to enable full support for AMD ROCm acceleration.
            If you observe problems when using GPU acceleration with AMD graphics card, enable <i>Override GPU version</i> option.</li>
        <li>Speech synchronized with subtitle timestamps. When the text format is set to <i>SRT Subtitles</i>,
            the generated speech will be synchronized with the subtitle timestamps.
            This can be useful if you want to make voice over.</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New model: Lithuanian to English</li>
        <li>Option to force text cleaning before translation.
            If the input text is incorrectly formatted, this option may improve the translation quality.
            To enable, use the switch to the right of <i>Translate</i> button.</li>
        <li>Text formatting support. The translation will preserve the formatting from the input text.
            Supported formats are: HTML, Markdown and SRT Subtitles.
            To enable, use the drop-down menu at the bottom of the text area.</li>
        <li>Translation progress indicator</li>
        </ul>
        <p>" + qsTr("Other") + "</p>
        <ul>
        <li>Setting option to override GPU version
            (<i>Other</i> &rarr; <i>Override GPU version</i>).
            Enable this option if you observe problems when using GPU acceleration with AMD graphics card.</li>
        <li>Setting option to limit number of simultaneous CPU threads
            (<i>Other</i> &rarr; <i>Number of simultaneous threads</i>)</li>
        <li>Setting option to set Python libraries directory
            (<i>Other</i> &rarr; <i>Location of Python libraries</i>).
            This option may be useful if you use <i>venv</i> module to manage Python libraries.
            Using this option only makes sense for non Flatpak installations.</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.3.0")
    }

    RichLabel {
        text: "<p>" + qsTr("Accessibility") + "</p>
        <ul>
        <li>Global keyboard shortcuts. Shortcuts allow you to start or stop listening and reading using keyboard.
            Keyboard shortcuts function even when the application is not active (e.g. minimized or in the background).
            You can enable and modify shortcuts in the settings (<i>User Interface</i> &rarr; <i>Use global keyboard shortcuts</i>).
            This feature only works under X11.</li>
        <li>Support for <i>Actions</i>. This feature allows external application to invoke certain operation
            when <i>Speech Note</i> is running. An action can be triggered via DBus call or with command-line
            option. You can enable <i>Actions</i> in the settings
            (<i>User Interface</i> &rarr; <i>Allow external applications to invoke actions</i>).</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Desktop notifications. By default, when <i>Speech Note</i> is in the background, desktop notifications
            are shown to indicate starting or ending of listening and reading. Desktop notifications are especially useful
            when you use <i>Actions</i> or global keyboard shortcuts.
            You can disable desktop notifications in the settings
            (<i>User Interface</i> &rarr; <i>Show desktop notification</i>).</li>
        <li>Opening files with Drag and Drop gesture.</li>
        <li>Speech speed control option has been moved to the main application window.</li>
        <li>Fix: Application did not use native widgets on some platforms.</li>
        </ul>
        <p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New model: English to Hungarian</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>New languages: Afrikaans, Gujarati, Hausa, Telugu, Tswana, Javanese, Hebrew</li>
        <li>New engine: <i>Faster Whisper</i>. It provides slightly better performance compared to the existing engine
            for Whisper models, especially on bigger models like Medium or Large.</li>
        <li>New engine: <i>April-asr</i>. It supports intermediate results and punctuation (only English).
            New engine comes with models for the following languages: English, French, Polish.</li>
        <li>Inserting text to any active window. Using global keyboard shotcut or
            <i>start-listening-active-window</i> action you can
            directly insert the decoded text into any window which is currently in focus.
            This feature only works under X11 and not for all applications.</li>
        <li>Copy text to the clipboard. Using global keyboard shotcut or
            <i>start-listening-clipboard</i> action, the decoded text can be copied to the clipboard
            instead of being inserted into the current note.</li>
        <li>Stop listening button. Unlike Cancel, with this button you can stop listening but
            the already recorded voice will be decoded into text.</li>
        <li>Support for Opus audio codec in <i>Transcribe a file</i></li>
        <li>More effective GPU acceleration for Whisper models. Average decoding time has been shortened by 3 times.</li>
        <li>New Whisper models for English: <i>Distil-Whisper Medium</i> and <i>Distil-Whisper Large-v2</i>.
            Both Distil models are faster than currently enabled <i>Whisper Medium</i> and <i>Whisper Large</i>.</li>
        <li>New version of Whisper Large model: <i>Whisper Large-v3</i></li>
        <li>Fix: CUDA acceleration for Whisper models did not work on NVIDIA graphic cards with Maxwell architecture.</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>New languages: Afrikaans, Gujarati, Hausa, Telugu, Tswana, Javanese, Hebrew</li>
        <li>New engine: <i>Mimic 3</i> with voices for the following languages:
            Afrikaans, Bengali, German, Greek, English, Spanish, Persian, Finnish,
            French, Gujarati, Hausa, Hungarian, Italian, Javanese, Nepali, Dutch,
            Polish, Russian, Swedish, Telugu, Tswana, Ukrainian, Yoruba.
        </li>
        <li>Reading text from the clipboard. Using global keyboard shotcut or
            <i>start-reading-clipboard</i> action you can directly read text that is in the clipboard.</li>
        <li>New <i>Piper</i> voices for the following languages: Arabic, English, Hungarian, Polish, Czech, German,
            Ukrainian, Vietnamese, Serbian, French, Spanish, Nepali.</li>
        <li>More steps in the <i>Speech speed</i> option. You can set speed from 0.1 to 2.0 values.</li>
        <li>Diacritical marks restoration before speech synthesis for Arabic and Hebrew.
            It is enabled by default,
            but if you don't need this option, it can be disabled in the settings.</li>
        <li>Support for GPU acceleration for <i>Coqui</i> models.
            If a suitable graphics card is found in the system, it will be used to accelerate processing.
            This significantly reduces the time of speech synthesis.
            GPU hardware acceleration is not enabled by default. Use the option in <i>Settings</i> to turn it on.
            Disable, if you observe any problems when using Text to Speech with <i>Coqui</i> models.
        </li>
        <li>Fix: <i>Coqui</i> Chinese MMS Hakka and Min Nan voices were broken.</li>
        <li>Fix: Exporting to audio file was not possible when text was very long.</li>
        </ul>
        <p>" + qsTr("Other") + "</p>
        <ul>
        <li>Setting option to disable support for certain graphic cards
            (<i>Other</i> &rarr; <i>Graphics cards support</i>). Use this option if you observe problems
            when using GPU acceleration. If the application does not start due to graphics card problem,
            use the <i>--gpu-scan-off</i> command line option to switch off any GPU support.</li>
        <li>Setting option <i>Clear cache on close</i></li>
        <li>Cache compression. Temporary audio files are stored in Opus format instead of raw audio.
            This significantly reduces the required disk space.</li>
        <li>Detecting the availability of the optional features. In the settings, you can check what optional features
            are available (<i>Other</i> &rarr; <i>Availability of optional features</i>).
            Feature availability depends on the libraries installed on your system.
            If the application does not start due to problem in Python library,
            use the <i>--py-scan-off</i> command line option to switch off any Python support.</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.2.1")
    }

    RichLabel {
        text: "<p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Improved AMD GPU acceleration support for <i>Whisper</i> models.
            Whisper GPU accelerator for AMD cards uses OpenCL interface.
            OpenCL implementation shipped in Flatpak runtime 'Clover'
            does not support new AMD cards.
            To overcome this problem, Speech Note package provides another
            implementation of OpenCL 'ROCm-OpenCL' which supports new hardware.
        </li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.2.0")
    }

    RichLabel {
        text: "<p>" + qsTr("Translator") + "</p>
        <ul>
        <li>New models: Hungarian to English, Finnish to English</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Support for video files transcription. With <i>Transcribe a file</i> menu option you can convert
            audio file or audio from video file to text.
            Following video formats are supported: MP4, MKV, Ogg.
        </li>
        <li>Option <i>Audio source</i> in <i>Settings</i> to select preferred audio source. New option let you choose
            microphone (or other audio source) which is used in Speech to Text.</li>
        <li>Whisper engine update. Library behind Whisper engine (whisper.cpp) has been updated resulting in
            an increase in performance. Processing time on CPU has been cut in half on average.</li>
        <li>Improved Nvidia GPU acceleration support for Whisper models.
            Following Whisper accelerators are currently enabled:
            OpenCL (for most Nvidia cards, few AMD cards and Intel GPUs),
            CUDA (for most Nvidia cards).
            Support for AMD ROCm is implemented as well but right now it doesn't work due to Flatpak sandboxing isolation.
            GPU hardware acceleration might not work well on your system, therefore is not enabled by default.
            Use the option in <i>Settings</i> to turn it on.
            Disable, if you observe any problems when using Speech to Text with Whisper models.</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>Save audio in compressed formats (MP3 or Ogg Vorbis). You can also save metadata tags to the audio file,
            such as track number, title, artist or album.</li>
        <li>Pause option. Note reading can be paused and resumed.</li>
        <li>New models from Massively Multilingual Speech (MMS) project: Hungarian, Catalan, German, Spanish,
            Romanian, Russian and Swedish.
            If you would like any other MMS model to be included, please let us know.</li>
        <li>Update of RHVoice voice for Uzbek.</li>
        <li>Fix: Many Coqui models couldn't read the numbers or the reading wasn't correct.</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Menu options: <i>Open a text file</i> and <i>Save to a text file</i></li>
        <li>Command line option to open files. If you want to associate text, audio or video files
            with Speech Note, now it is possible. Your system may detect this new capability and
            show Speech Note under <i>Open With</i> menu in the file manager.
            Please note that Flatpak app only has permission to access files in the following folders:
            Desktop, Documents, Downloads, Music and Videos.</li>
        <li>Improved UI colors when app is running under GNOME dark theme.</li>
        <li>Advanced settings option <i>Graphical style</i>. This option let you select any
            Qt interface style installed in your system. Changing the style might make app
            look better under GNOME.</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.1.0")
    }

    RichLabel {
        text: "<p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Support for GPU acceleration for Whisper models.
            If a suitable graphics card is found in the system, it will be used to accelerate processing.
            This significantly reduces the time of decoding (usually 2 times or more).
            GPU hardware acceleration is not enabled by default. Use the option in <i>Settings</i> to turn it on.
            Disable, if you observe any problems when using Speech to Text with Whisper models.
        </li>
        <li>Fix: Whisper model wasn't able to decode short speech sentences.</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>Option <i>Speech speed</i> in <i>Settings</i> to make synthesized speech slower or faster.</li>
        <li>New models from <a href='https://ai.meta.com/blog/multilingual-model-speech-recognition'>Massively Multilingual Speech (MMS)</a> project.
            MMS project released models for <a href='https://dl.fbaipublicfiles.com/mms/tts/all-tts-languages.html'>1100 languages</a>,
            but only the following have been enabled:
            Albanian, Amharic, Arabic, Basque, Bengali, Bulgarian, Chinese, Greek, Hindi, Icelandic, Indonesian,
            Kazakh, Korean, Latin, Latvian, Malay, Mongolian, Polish, Portuguese, Swahili, Tagalog, Tatar, Thai,
            Turkish, Uzbek, Vietnamese and Yoruba.
            If you would like any other MMS model to be included, please let us know.</li>
        <li>New Coqui voices for: Japanese, Turkish and Spanish.</li>
        <li>New Piper voices for: Czech, German, Hungarian, Portuguese, Slovak and English.</li>
        <li>Update of RHVoice voices for Slovak and Czech.</li>
        <li>Fix: Splitting text into sentences was incorrect for: Georgian, Japanese, Bengali, Nepali and Hindi.</li>
        </ul>
        <p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Option to change font size in text editor</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("4.0.0")
    }

    RichLabel {
        text: "<p>" + qsTr("Translator") + "</p>
              <ul>
              <li>Support for offline translations between following languages: Catalan, Bulgarian, Czech, Danish,
                  English, Spanish, German, Estonian, French, Italian, Polish, Portuguese, Norwegian, Iranian, Dutch,
                  Russian, Ukrainian, Icelandic.</li>
              <li><i>Translator</i> uses models that were created as part of <a href='https://browser.mt/'>Bergamot project</a>.</li>
              <li>To switch between <i>Notepad</i> and <i>Translator</i> modes, use the toggle buttons in the upper right corner.</li>
              </ul>
              <p>" + qsTr("User Interface") + "</p>
              <ul>
              <li>User interface has been redesign. It is more handy and better supports portrait view for mobile.</li>
              <li>Settings option to force specific <i>Interface style</i> has been added. It is useful to overcome UI glitches when app is running under GNOME desktop environment.</li>
              <li>Application has been translated to new languages: Dutch and Italian.</li>
              </ul>
              <p>" + qsTr("Text to Speech") + "</p>
              <ul>
              <li>All existing Piper models have been updated.</li>
              <li>New Piper voices for: English, Swedish, Turkish, Polish, German, Spanish, Finnish, French, Ukrainian, Russian,
                    Swahili, Serbian, Romanian, Luxembourgish and Georgian</li>
              <li>New RHVoice model for Slovak language</li>
              </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("3.1.5")
    }

    RichLabel {
        text: "<p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>New Coqui voice for English: Jenny</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Quicker decoding when using DeepSpeech/Coqui models (especially on ARM CPU)</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("3.1.4")
    }

    RichLabel {
        text: "<p>" + qsTr("User Interface") + "</p>
        <ul>
        <li>Option to show recent changes in the app (About -> Changes)</li>
        <li>French translation update (Many thanks to L'Africain)</li>
        </ul>
        <p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>New Piper model for Chinese</li>
        <li>New RHVoice model for Uzbek</li>
        <li>Updated RHVoice models for Ukrainian</li>
        <li>Piper and RHVoice engines updated to most recent versions</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Whisper 'Large' models enabled for all languages</li>
        <li>Whisper supported on older CPUs (i.e. CPUs without AVX/AVX2 extensions)</li>
        <li>Whisper engine update (20% performance improvement, 50% less memory)</li>
        </ul>"
    }

    SectionLabel {
        text: qsTr("Version %1").arg("3.1.3")
    }

    RichLabel {
        text: "<p>" + qsTr("Text to Speech") + "</p>
        <ul>
        <li>New Piper models for: Icelandic, Swedish and Russian</li>
        </ul>
        <p>" + qsTr("Speech to Text") + "</p>
        <ul>
        <li>Whisper fine-tuned models for: Czech, Slovak, Slovenian, Romanian, Russian, Hungarian and Polish</li>
        <li>Standard Whisper models enabled also for:
            Amharic, Arabic, Bengali, Danish, Estonian, Basque, Persian, Hindi,
            Croatian, Hungarian, Icelandic, Georgian,
            Kazakh, Korean, Lithuanian, Latvian, Mongolian,
            Maltese, Nepali, Romanian, Slovak, Slovenian, Albanian,
            Swahili, Tagalog, Tatar, Uzbek and Yoruba</li>
        </ul>"
    }
}
