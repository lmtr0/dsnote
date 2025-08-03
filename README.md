# Speech Note

Linux desktop and Sailfish OS app for note taking, reading and translating with offline Speech to Text, Text to Speech and Machine Translation

<a href='https://flathub.org/apps/net.mkiol.SpeechNote'><img width='240' alt='Download on Flathub' src='https://dl.flathub.org/assets/badges/flathub-badge-en.png'/></a>

## Contents of this README

- [Description](#description)
- [Languages and Models](#languages-and-models)
- [How to install](#how-to-install)
- [Flatpak packages](#flatpak-packages)
- [Beta version](#beta-version)
- [Extra features](#extra-features)
- [Building from sources](#building-from-sources)
- [How to enable a custom model](#how-to-enable-a-custom-model)
- [Contributing to Speech Note](#contributing-to-speech-note)
- [How to support](#how-to-support)
- [Reviews and demos](#reviews-and-demos)
- [License](#license)

## Description

**Speech Note** let you take, read and translate notes in multiple languages.
It uses Speech to Text, Text to Speech and Machine Translation to do so.
Text and voice processing take place entirely offline, locally on your
computer, without using a network connection. Your privacy is always
respected. No data is sent to the Internet.

**Speech Note** uses many different processing engines to do its job.
Currently these are used:

- Speech to Text (STT)
    - [Coqui STT (a fork of Mozilla DeepSpeech)](https://github.com/coqui-ai/STT)
    - [Vosk](https://alphacephei.com/vosk)
    - [whisper.cpp](https://github.com/ggerganov/whisper.cpp)
    - [Faster Whisper](https://github.com/guillaumekln/faster-whisper)
    - [april-asr](https://github.com/abb128/april-asr)
- Text to Speech (TTS)
    - [espeak-ng](https://github.com/espeak-ng/espeak-ng)
    - [MBROLA](https://github.com/numediart/MBROLA)
    - [Piper](https://github.com/rhasspy/piper)
    - [RHVoice](https://github.com/RHVoice/RHVoice)
    - [Coqui TTS](https://github.com/coqui-ai/TTS)
    - [Mimic 3](https://mycroft.ai/mimic-3)
    - [WhisperSpeech](https://collabora.github.io/WhisperSpeech/)
    - [Kokoro](https://github.com/hexgrad/kokoro)
    - [Parler-TTS](https://github.com/huggingface/parler-tts)
    - [F5-TTS](https://github.com/SWivid/F5-TTS)
    - [S.A.M.](https://discordier.github.io/sam)
- Machine Translation (MT)
    - [Bergamot Translator](https://github.com/browsermt/bergamot-translator)

## Languages and Models

Speech Note installation package does not include checkpoint files for supported models, but instead they can be easily downloaded using the graphical model browser built into the application.

Following languages and models are supported and enable for download:

| **Lang ID** | **Name**      | **DeepSpeech (STT)** | **Whisper (STT)** | **Vosk (STT)** | **April-ASR (STT)** | **Piper (TTS)** | **RHVoice (TTS)** | **espeak (TTS)** | **MBROLA (TTS)** | **Coqui (TTS)** | **Mimic3 (TTS)** | **WhisperSpeech (TTS)** | **Kokoro (TTS)** | **F5-TTS** | **Parler-TTS** | **S.A.M. (TTS)** | **Bergamot (MT)** |
| ----------- | ------------- | -------------------- | ----------------- | -------------- | ------------------- | --------------- | ----------------- | ---------------- | ---------------- | --------------- | ---------------- | ----------------------- | ---------------- | ---------- | -------------- | ---------------- | ----------------- |
| af          | Afrikaans     |                      | ●                 |                |                     |                 |                   | ●                |                  |                 | ●                |                         |                  |            |                |                  |                   |
| am          | Amharic       | ● (e)                | ●                 |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| ar          | Arabic        |                      | ●                 | ●              |                     | ●               |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| az          | Azerbaijani   |                      | ●                 |                |                     |                 |                   |                  |                  |                 |                  |                         |                  |            |                |                  | ●                 |
| be          | Belarusian    |                      | ●                 |                |                     |                 |                   |                  |                  |                 |                  |                         |                  |            |                |                  | ●                 |
| bg          | Bulgarian     |                      | ●                 |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| bn          | Bengali       |                      | ●                 |                |                     |                 |                   | ●                |                  | ●               | ●                |                         |                  |            |                |                  |                   |
| bs          | Bosnian       |                      | ●                 |                |                     |                 |                   | ●                |                  |                 |                  |                         |                  |            |                |                  | ●                 |
| ca          | Catalan       | ●                    | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| cs          | Czech         | ●                    | ●                 | ●              |                     | ●               | ●                 | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| cy          | Welsh         |                      |                   |                |                     | ●               |                   |                  |                  |                 |                  |                         |                  |            |                |                  |                   |
| da          | Danish        |                      | ●                 |                |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| de          | German        | ●                    | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               | ●                | ●                       |                  |            | ●(e)           |                  | ●                 |
| el          | Greek         | ● (e)                | ●                 |                |                     | ●               |                   | ●                |                  | ●               | ●                |                         |                  |            |                |                  | ●                 |
| en          | English       | ●                    | ●                 | ●              | ●                   | ●               | ●                 | ●                |                  | ●               | ●                | ●                       | ●                | ●          | ●              | ●                | ●                 |
| eo          | Esperanto     |                      |                   | ●              |                     |                 | ●                 | ●                |                  |                 |                  |                         |                  |            |                |                  |                   |
| es          | Spanish       | ●                    | ●                 | ●              |                     | ●               | ●                 | ●                |                  | ●               | ●                | ●                       | ●                |            | ●(e)           |                  | ●                 |
| et          | Estonian      | ● (e)                | ●                 |                |                     |                 |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| eu          | Basque        | ● (e)                | ●                 |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| fa          | Persian       | ●                    | ●                 | ●              |                     | ●               |                   | ●                | ●                | ●               | ●                |                         |                  |            |                |                  | ●                 |
| fi          | Finnish       | ●                    | ●                 |                |                     | ●               |                   | ●                |                  | ●               | ●                |                         |                  |            |                |                  | ●                 |
| fr          | French        | ●                    | ●                 | ●              | ●                   | ●               |                   | ●                |                  | ●               | ●                | ●                       | ●                |            | ●(e)           |                  | ●                 |
| ga          | Irish         |                      |                   |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| gu          | Gujarati      |                      | ●                 |                |                     |                 |                   | ●                |                  |                 | ●                |                         |                  |            |                |                  |                   |
| ha          | Hausa         |                      | ●                 |                |                     |                 |                   |                  |                  |                 | ●                |                         |                  |            |                |                  |                   |
| he          | Hebrew        |                      | ●                 |                |                     |                 |                   |                  |                  | ●               |                  |                         |                  |            |                |                  |                   |
| hi          | Hindi         |                      | ●                 | ●              |                     | ●               |                   | ●                |                  |                 |                  |                         | ●                |            |                |                  |                   |
| hr          | Croatian      |                      | ●                 |                |                     |                 | ●                 | ●                | ●                | ●               |                  |                         |                  |            |                |                  |                   |
| hu          | Hungarian     | ● (e)                | ●                 |                |                     | ●               |                   | ●                | ●                | ●               | ●                |                         |                  |            |                |                  | ●                 |
| id          | Indonesian    | ● (e)                | ●                 |                |                     |                 |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| is          | Icelandic     |                      | ●                 |                |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| it          | Italian       | ●                    | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               | ●                | ●                       | ●                |            | ●(e)           |                  | ●                 |
| ja          | Japanese      |                      | ●                 | ●              |                     |                 |                   | ●                |                  | ●               |                  |                         | ●                |            |                |                  | ●                 |
| jv          | Javanese      |                      | ●                 |                |                     |                 |                   |                  |                  |                 | ●                |                         |                  |            |                |                  |                   |
| ka          | Georgian      |                      | ●                 |                |                     | ●               | ●                 | ●                |                  |                 |                  |                         |                  |            |                |                  |                   |
| kk          | Kazakh        |                      | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| kn          | Kannada       |                      | ●                 |                |                     |                 |                   |                  |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| ko          | Korean        |                      | ●                 | ●              |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| ky          | Kyrgyz        |                      |                   |                |                     |                 | ●                 | ●                |                  |                 |                  |                         |                  |            |                |                  |                   |
| la          | Latin         |                      |                   |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| lb          | Luxembourgish |                      |                   |                |                     | ●               |                   |                  |                  |                 |                  |                         |                  |            |                |                  |                   |
| lt          | Lithuanian    |                      | ●                 |                |                     |                 |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| lv          | Latvian       | ●                    | ●                 |                |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| mk          | Macedonian    |                      | ●                 |                |                     |                 | ●                 | ●                |                  |                 |                  |                         |                  |            |                |                  |                   |
| ml          | Malayalam     |                      | ● (e)             |                |                     | ●               |                   |                  |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| mn          | Mongolian     | ● (e)                | ●                 |                |                     |                 |                   |                  |                  | ●               |                  |                         |                  |            |                |                  |                   |
| mr          | Marathi       |                      | ●                 |                |                     |                 |                   |                  |                  | ●               |                  |                         |                  |            |                |                  |                   |
| ms          | Malay         |                      | ●                 |                |                     |                 |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  |                   |
| mt          | Maltese       |                      | ●                 |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| ne          | Nepali        |                      | ●                 |                |                     | ●               |                   | ●                |                  |                 | ●                |                         |                  |            |                |                  |                   |
| nl          | Dutch         | ● (e)                | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               | ●                | ●                       |                  |            | ●(e)           |                  | ●                 |
| no          | Norwegian     |                      | ●                 |                |                     | ●               |                   | ●                |                  |                 |                  |                         |                  |            |                |                  | ●                 |
| pl          | Polish        | ●                    | ●                 | ●              | ●                   | ●               | ●                 | ●                | ●                | ●               | ●                | ●                       |                  |            | ●(e)           |                  | ●                 |
| pt          | Portuguese    | ● (e)                | ●                 | ●              |                     | ●               |                   | ●                | ●                | ●               |                  |                         | ●                |            | ●(e)           |                  | ●                 |
| ro          | Romanian      | ● (e)                | ●                 |                |                     | ●               |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| ru          | Russian       | ●                    | ●                 | ●              |                     | ●               | ●                 | ●                |                  |                 | ●                |                         |                  |            |                |                  | ●                 |
| sk          | Slovak        |                      | ●                 |                |                     | ●               | ●                 | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| sl          | Slovenian     | ● (e)                | ●                 |                |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| sq          | Albanian      |                      | ●                 |                |                     |                 | ●                 | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| sr          | Serbian       |                      | ●                 |                |                     | ●               | ●                 | ●                |                  |                 |                  |                         |                  |            |                |                  | ●                 |
| sv          | Swedish       |                      | ●                 | ●              |                     | ●               |                   | ●                | ●                | ●               | ●                |                         |                  |            |                |                  | ●                 |
| sw          | Swahili       | ●                    | ●                 |                |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| ta          | Tamil         |                      | ●                 |                |                     |                 |                   |                  |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| te          | Telugu        |                      | ●                 |                |                     |                 |                   | ●                |                  |                 | ●                |                         |                  |            |                |                  |                   |
| th          | Thai          | ● (e)                | ●                 |                |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| tl          | Tagalog       |                      | ●                 | ●              |                     |                 |                   |                  |                  | ●               |                  |                         |                  |            |                |                  |                   |
| tn          | Tswana        |                      | ●                 |                |                     |                 |                   | ●                |                  |                 | ●                |                         |                  |            |                |                  |                   |
| tr          | Turkish       | ● (e)                | ●                 | ●              |                     | ●               |                   | ●                | ●                | ●               |                  |                         |                  |            |                |                  | ●                 |
| tt          | Tatar         |                      | ●                 |                |                     |                 | ●                 | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| uk          | Ukrainian     | ●                    | ●                 | ●              |                     | ●               | ●                 | ●                |                  | ●               | ●                |                         |                  |            |                |                  | ●                 |
| uz          | Uzbek         |                      | ●                 | ●              |                     |                 |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  |                   |
| vi          | Vietnamese    |                      | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               |                  |                         |                  |            |                |                  | ●                 |
| yo          | Yoruba        | ● (e)                | ●                 |                |                     |                 |                   |                  |                  | ●               | ●                |                         |                  |            |                |                  |                   |
| zh          | Chinese       | ●                    | ●                 | ●              |                     | ●               |                   | ●                |                  | ●               |                  |                         | ●                | ●          |                |                  | ●                 |

<sup>(e) experimental, most likely doesn't work well</sup>
<br/>

Faster Whisper, Coqui TTS and Mimic3 models are only available on x86-64.

Language models can be downloaded directly from the app.

Details of models which are currently configured for download are described in
[models.json (GitHub)](https://github.com/mkiol/dsnote/blob/main/config/models.json) or
[models.json (GitLab)](https://gitlab.com/mkiol/dsnote/-/blob/main/config/models.json).

## How to install

- Linux Desktop: [Flatpak](https://flathub.org/apps/net.mkiol.SpeechNote)

```sh
# Flatpak base package
flatpak install net.mkiol.SpeechNote

# Optional NVIDIA add-on package
flatpak install net.mkiol.SpeechNote.Addon.nvidia

# Optional AMD add-on package
flatpak install net.mkiol.SpeechNote.Addon.amd
```

- Arch Linux (AUR):

  - [dsnote](https://aur.archlinux.org/packages/dsnote)
  - [dsnote-git](https://aur.archlinux.org/packages/dsnote-git)

- openSUSE ([Packman repository](https://en.opensuse.org/Additional_package_repositories#Packman))

```sh
# Base package
zypper in speechnote

# Optional support for Python-based features in Speech Note
zypper in speechnote-python-modules
```

- Sailfish OS: [OpenRepos](https://openrepos.net/content/mkiol/speech-note)

### Flatpak packages

The app distributed via Flatpak (published on Flathub) consists of the following packages:

 - Base package "Speech Note" (net.mkiol.SpeechNote)
 - Optional add-on for NVIDIA graphics card "Speech Note NVIDIA" (net.mkiol.SpeechNote.Addon.nvidia)
 - Optional add-on for AMD graphics card "Speech Note AMD" (net.mkiol.SpeechNote.Addon.amd)

Base package includes all the dependencies needed to run every feature of the application.
Add-ons add the capability of GPU acceleration, which speeds up some operations in the application.

Base package and add-ons contain many "heavy" libraries like CUDA, ROCm, Torch and Python libraries.
Due to this, the size of the packages and the space required after installation are significant.
If you don't need all the functionalities, you can use much smaller "Tiny" package
(available on [Releases](https://github.com/mkiol/dsnote/releases) page),
which provides only the basic features. If you need, you can also use "Tiny" packages together with GPU acceleration add-on.

Size comparison between Base, Tiny and Add-ons Flatpak packages:

| **Sizes**     | **Base** | **Tiny** | **AMD add-on** | **NVIDIA add-on** |
| ------------- | ---------| ---------| -------------- | ----------------- |
| Download size | 1.2 GiB  |  48 MiB  |  +7.6 GiB      | +4.3 GiB          |
| Unpacked size | 3.6 GiB  | 170 MiB  |  +34 GiB       | +9 GiB            |

Add-on packages use so-called "extra-data". This means that during installation, several binary modules are downloaded from the original sources and merged with the Flatpak package. Due to this process, additional disk space is temporarily required when installing the add-on. Make sure you have enough disk space on the partition where flatpak applications are installed. When the Flatpak package is installed system-wide (default), the applications are stored in _/var/lib/flatpak_. In case of user installation (--user), the apps are stored in _~/.local/share/flatpak_.

Disk space required for the installation of the add-on package:

| **Add-on**    | **Size** |
| --------------| ---------|
| AMD add-on    | 55 GiB   |
| NVIDIA add-on | 15 GiB   |

Features comparison between Base, Tiny and Add-ons Flatpak packages:

| **Features**                            | **Base** | **Tiny** | **AMD add-on** | **NVIDIA add-on** |
| --------------------------------------- | ---------| ---------| -------------- | ----------------- |
| Coqui/DeepSpeech STT                    | +        | +        |                |                   |
| Vosk STT                                | +        | +        |                |                   |
| Whisper (whisper.cpp) STT               | +        | +        |                |                   |
| Whisper (whisper.cpp) STT OpenCL ROCm   | -        | -        | +              |                   |
| Whisper (whisper.cpp) STT OpenCL NVIDIA | +        | +        |                |                   |
| Whisper (whisper.cpp) STT ROCm          | -        | -        | +              |                   |
| Whisper (whisper.cpp) STT CUDA          | -        | -        |                | +                 |
| Whisper (whisper.cpp) STT OpenVINO      | +        | -        |                |                   |
| Whisper (whisper.cpp) STT Vulkan        | +        | +        |                |                   |
| FasterWhisper STT                       | +        | -        |                |                   |
| FasterWhisper STT CUDA                  | -        | -        |                | +                 |
| April-ASR STT                           | +        | +        |                |                   |
| eSpeak TTS                              | +        | +        |                |                   |
| MBROLA TTS                              | +        | +        |                |                   |
| Piper TTS                               | +        | +        |                |                   |
| RHVoice TTS                             | +        | +        |                |                   |
| Coqui TTS                               | +        | -        |                |                   |
| Coqui TTS ROCm                          | -        | -        | +              |                   |
| Coqui TTS CUDA                          | -        | -        |                | +                 |
| Mimic3 TTS                              | +        | -        |                |                   |
| WhisperSpeech TTS                       | +        | -        |                |                   |
| WhisperSpeech TTS ROCm                  | -        | -        | +              |                   |
| WhisperSpeech TTS CUDA                  | -        | -        |                | +                 |
| Kokoro TTS                              | +        | -        |                |                   |
| Kokoro TTS ROCm                         | -        | -        | +              |                   |
| Kokoro TTS CUDA                         | -        | -        |                | +                 |
| Parler-TTS                              | +        | -        |                |                   |
| Parler-TTS ROCm                         | -        | -        | +              |                   |
| Parler-TTS CUDA                         | -        | -        |                | +                 |
| F5-TTS                                  | +        | -        |                |                   |
| F5-TTS ROCm                             | -        | -        | +              |                   |
| F5-TTS CUDA                             | -        | -        |                | +                 |
| S.A.M TTS                               | +        | +        |                |                   |
| Punctuation restoration                 | +        | -        |                |                   |
| Translator                              | +        | +        |                |                   |

### Beta version

In addition to the stable version in the Flathub repository, you can try to test the "Beta" version of the upcoming release. This version is usable, but may contain more bugs.

Beta version is available in "flathub-beta" repository. Follow [these instructions](https://discourse.flathub.org/t/how-to-use-flathub-beta/2111) to enable flathub-beta on your computer.

## Extra features

### Command-line options

The CLI interface is primary for the integration with the desktop when _Speech Note_ is already running (for example, hidden in the system tray or in the backgraund).

#### Examples

List all supported options:

```shell
flatpak run net.mkiol.SpeechNote --help
```

Start listening:

```sh
flatpak run net.mkiol.SpeechNote --action start-listening
```

Cancel any already started action:

```sh
flatpak run net.mkiol.SpeechNote --action cancel
```

Start listening, the decoded text will be saved to the clipboard:

```sh
flatpak run net.mkiol.SpeechNote --action start-listening-clipboard
```

Start listening, the decoded text will be inserted into any window on the desktop on which the cursor is focused:

```sh
flatpak run net.mkiol.SpeechNote --action start-listening-active-window
```

Start reading "Hello, how are you doing?":

```sh
flatpak run net.mkiol.SpeechNote --action start-reading-text --text "Hello, how are you doing?"
```

Save speech of "Hello, how are you doing?" to "speech.mp3" file:

```sh
flatpak run net.mkiol.SpeechNote --action start-reading-text --text "Hello, how are you doing?" --output-file speech.mp3
```

List all available TTS models:

```sh
flatpak run net.mkiol.SpeechNote --print-available-models tts
```

### Global keyboard shortcuts

Global keyboard shortcuts allow you to start listening or reading with the keyboard, even when the application is not active (e.g. minimized, hidden in the system tray icon or just in the background).

To enable and customize keyboard shortcuts, go to `Settings`->`Accessibility`->`Use Global Keyboard Shortcuts`.

In order for shortcuts to work under Wayland, your desktop environment must support [GlobalShortcuts interface](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.impl.portal.GlobalShortcuts.html) in the _XDG Desktop Portal_ service. Currently, `GlobalShortcuts` is supported only in the latest KDE Plasma and GNOME desktops.

When `XDG Desktop Portal` is used to manage global shortcuts, use the desktop environment tool to customize key bindings.

### Insert into active window

Using global keyboard shortcuts or command-line actions, you can directly start listening and insert the decoded text into any window that is currently in focus. This allows you to use _Speech Note_ as a voice typing tool on the desktop.

Under X11, this feature should work right out of the box.

Under Wayland, the external `ydotool` daemon must be installed and running for it to work. If you are using Flatpak, also make sure that the application has permission to access the `ydotool` daemon's socket file and the socket file.

### Subtitles

When transcribing (STT), you can also convert audio to subtitles instead of text. The supported format is SRT.

Subtitles are also supported in TTS. The reading aloud of subtitles is synchronized with the timestamps of the subtitles. If configured, the speed of the voice can be automatically adjusted to the duration of the subtitle segment. This allows you to create a voice-over from the subtitle file.

## Building from sources

### Arch Linux

It is also possible to build and install the latest development (git) or latest stable (release) version from the repository using the provided PKGBUILD file (please note that the same remarks about building on Linux apply):

```sh
git clone <git repository url>

cd dsnote/arch/git      # build latest git version
# or
cd dsnote/arch/release  # build latest release version

makepkg -si
```

### RHEL/Fedora/Rocky Linux

It is also possible to build and install the latest development version from the repository using the provided SPEC file and helper `make_rpm.sh` script:

```sh
git clone <git repository url>

cd dsnote/fedora

# optionally install build dependencies
dnf install rpmdevtools autoconf automake boost-devel cmake git kf5-kdbusaddons-devel libarchive-devel libxdo-devel libXinerama-devel libxkbcommon-x11-devel libXtst-devel libtool meson openblas-devel patchelf pybind11-devel python3-devel python3-pybind11 qt5-linguist qt5-qtmultimedia-devel qt5-qtquickcontrols2-devel qt5-qtx11extras-devel rubberband-devel taglib-devel vulkan-headers

./make_rpm.sh
```

### Flatpak

```sh
git clone <git repository url>

cd dsnote/flatpak

# build a base package
flatpak-builder --force-clean --user --install-deps-from=flathub --repo="<name or /path/to/local/flatpak/repo>" "/path/to/output/dir" net.mkiol.SpeechNote.yaml

# build an optional NVIDIA add-on package
flatpak-builder --force-clean --user --install-deps-from=flathub --repo="<name or /path/to/local/flatpak/repo>" "/path/to/output/dir" net.mkiol.SpeechNote.Addon.nvidia.yaml
```

### Sailfish OS

```sh
git clone <git repository url>

cd dsnote
mkdir build
cd build

sfdk config --session specfile=../sfos/harbour-dsnote.spec
sfdk config --session target=SailfishOS-4.4.0.58-aarch64
sfdk cmake ../ -DCMAKE_BUILD_TYPE=Release -DWITH_SFOS=ON -DWITH_PY=OFF
sfdk package
```

### Linux (direct build)

Speech Note has many build-time and run-time dependencies. This includes shared and static libraries,
3rd-party executables, Python and Perl scripts. Because of these complexity, the recommended way to build
is to use Flatpak tool-chain (Flatpak manifest file and [flatpak-builder](https://docs.flatpak.org/en/latest/flatpak-builder.html)).
If you want to make a direct build (i.e. without flatpak) it is also possible but more complicated.

```sh
git clone <git repository url>

cd dsnote
mkdir build
cd build

cmake ../ -DCMAKE_BUILD_TYPE=Release -DWITH_DESKTOP=ON
make
```

To make build without support for Python components, add `-DWITH_PY=OFF` in cmake step.

To see other build options search for `option(BUILD_XXX)` in `CMakeList.txt` file.

## How to enable a custom model

All models available for download are specified in the configuration file (config/models.json).
To enable a custom model that is compatible with currently supported engines, simply edit this file and restart the application.

When you first run the application, the models configuration file is created in:

- `~/.local/share/net.mkiol/dsnote/models.json`, or
- `~/.var/app/net.mkiol.SpeechNote/data/net.mkiol/dsnote/models.json` (Flatpak), or
- `~/.local/share/org.mkiol/dsnote/models.json` (Sailfish OS)

You can freely edit currently enabled models or add new ones.

Model definition looks like this:

```
{
    "name": "<model name>",
    "model_id": "<model unique id>",
    "engine": "<engine type>",
    "lang_id": "<lang id>",
    "checksum": "<md5 checksum>",
    "checksum_quick": "<partial md5 checksum>",
    "comp": "<compression type",
    "urls": [
        <model URLs>
    ],
    "size": "<download size of all files>"
}
```

Allowed engine types: `stt_ds`, `stt_vosk`, `stt_april`, `stt_whisper`, `stt_fasterwhisper`, `tts_piper`, `tts_rhvoice`, `tts_espeak`, `tts_coqui`, `tts_mimic3`, `tts_whisperspeech`, `tts_sam`, `tts_parler`, `tts_f5`, `tts_kokoro`, `mnt_bergamot`

Allowed compression types: `none`, `gz`, `xz`, `tarxz`, `targz`, `zip`, `zipall`, `dir`, `dirgz`

Allowed URL types: `http`, `https`, `file`

Checksums are calculated for all files after unpacking. If you are adding a new model, you can use the `--gen-checksums` command line option to find the right checksums. To do this, put empty strings in both `checksum` and `checksum_quick`, save the file and run Speech Note with the mentioned option.

For example:

```
{
    "name": "New Piper Voice",
    "model_id": "en_piper_new",
    "engine": "tts_piper",
    "lang_id": "en",
    "checksum": "",
    "checksum_quick": "",
    "size": ""
    "comp": "dir",
    "urls": [
        "file:///home/me/models/new-model-medium.onnx",
        "file:///home/me/models/new-model-medium.onnx.json"
    ]
}
```

```sh
flatpak run net.mkiol.SpeechNote --verbose --gen-checksums
```

## Contributing to Speech Note

Any contribution is very welcome!

Project is hosted both on [GitHub](https://github.com/mkiol/dsnote) and [GitLab](https://gitlab.com/mkiol/dsnote).
Feel free to make a PR/MR, report an issue or reqest for new feature on the platform you prefer the most.

### Translation

Translation files in Qt format are in `translations` directory.

Preferred way to contribute translation is via [Transifex service](https://explore.transifex.com/mkiol/dsnote/),
but if you would like to make a direct PR/MR, please do it.

## How to support

If you find **Speech Note** useful and would like to support this project,
please consider doing one or two of the following:

- Give a &#11088; on [GitHub](https://github.com/mkiol/dsnote) or/and [GitLab](https://gitlab.com/mkiol/dsnote).
- Write a review in your applications manager app (Discover, Software or any other).
- Tell others about this app by mentioning it on social media.
- If you have spare money, make a small donation via [ko-fi (one time)](https://ko-fi.com/mkiol) or [Liberapay (recurring)](https://liberapay.com/mkiol/donate).

## Libraries

**Speech Note** relies on following open source projects:

- [Qt](https://www.qt.io/)
- [Coqui STT](https://github.com/coqui-ai/STT)
- [Coqui TTS](https://github.com/coqui-ai/TTS)
- [Vosk](https://alphacephei.com/vosk)
- [whisper.cpp](https://github.com/ggerganov/whisper.cpp)
- [WebRTC VAD](https://webrtc.org/)
- [libarchive](https://libarchive.org/)
- [RNNoise-nu](https://github.com/GregorR/rnnoise-nu)
- [{fmt}](https://fmt.dev)
- [Hugging Face Transformers](https://github.com/huggingface/transformers)
- [Piper](https://github.com/rhasspy/piper)
- [RHVoice](https://github.com/RHVoice/RHVoice)
- [ssplit-cpp](https://github.com/ugermann/ssplit-cpp)
- [espeak-ng](https://github.com/espeak-ng/espeak-ng)
- [bergamot-translator](https://github.com/browsermt/bergamot-translator)
- [Rubber Band Library](https://breakfastquay.com/rubberband)
- [simdjson](https://simdjson.org/)
- [Nlohmann JSON](https://json.nlohmann.me/)
- [uroman](https://github.com/isi-nlp/uroman)
- [astrunc](https://github.com/Joke-Shi/astrunc)
- [FFmpeg](https://ffmpeg.org/)
- [LAME](https://lame.sourceforge.io/)
- [Vorbis](https://xiph.org/vorbis/)
- [TagLib](https://taglib.org/)
- [libnumbertext](https://github.com/Numbertext/libnumbertext)
- [KDBusAddons](https://invent.kde.org/frameworks/kdbusaddons)
- [QHotkey](https://github.com/Skycoder42/QHotkey)
- [faster-whisper](https://github.com/guillaumekln/faster-whisper)
- [Mimic 3](https://mycroft.ai/mimic-3)
- [Unikud](https://github.com/morrisalp/unikud)
- [april-asr](https://github.com/abb128/april-asr)
- [Opus](https://opus-codec.org/)
- [html2md](https://tim-gromeyer.github.io/html2md/)
- [maddy](https://github.com/progsource/maddy)
- [WhisperSpeech](https://collabora.github.io/WhisperSpeech/)
- [libxdo](https://github.com/jordansissel/xdotool)
- [Kokoro](https://github.com/hexgrad/kokoro)
- [Parler-TTS](https://github.com/huggingface/parler-tts)
- [F5-TTS](https://github.com/SWivid/F5-TTS)
- [S.A.M.](https://discordier.github.io/sam)

## Reviews and demos

- [Speech Note 4.8 changes video](https://www.youtube.com/watch?v=ww6skKOOzZ8) (Speech Note 4.8)
- [Speech Note 4.7 changes video](https://www.youtube.com/watch?v=cEht4Fts6Bo) (Speech Note 4.7)
- [Speech Note 4.6 changes video](https://www.youtube.com/watch?v=AVW5OY63wjg) (Speech Note 4.6)
- [Speech Note 4.5 changes video](https://youtu.be/S9MJ7y8-bcw) (Speech Note 4.5)
- [Screenshots](https://gitlab.com/mkiol/dsnote/-/tree/main/desktop/screenshots) (Speech Note 4.5)

- [Thejesh GN blog](https://thejeshgn.com/2025/05/26/speech-note-app-for-offline-speech-to-text-tts-and-translation/) (Speech Note 4.7)
- [LinuxD0 video](https://www.youtube.com/watch?v=OCnCY49CmDQ) (Speech Note 4.7, Spanish)
- [Guia Linux video](https://www.youtube.com/watch?v=_rL3II3JHNc) (Speech Note 4.7, Portuguese)
- [lwn.net](https://lwn.net/Articles/987315/) (Speech Note 4.6)
- [Softpedia](https://linux.softpedia.com/get/Utilities/Speech-Note-104828.shtml) (Speech Note 4.6)
- [OSTechNix](https://ostechnix.com/speech-note-speech-recognition-text-to-speech-translation-app-for-linux/) (Speech Note 4.6)
- [Best FREE Speech-to-Text For Linux Mint video](https://www.youtube.com/watch?v=VDMbWUfHsbk) (Speech Note 4.6)
- [Marco's Box](https://www.marcosbox.org/2024/02/speech-note-trascrivi-e-traduci-offline-.html) (Speech Note 4.4, Italian)
- [Marco's Box video](https://www.youtube.com/watch?v=6fNgZlh-O-w) (Speech Note 4.4, Italian)
- [alternativalinux](https://www.alternativalinux.it/riconoscimento-sintesi-vocale-e-traduttore-per-linux/) (Speech Note 4.4, Italian)
- [alternativalinux video](https://www.youtube.com/watch?v=6Peoss66fMg) (Speech Note 4.4, Italian)
- [ZDNET](https://www.zdnet.com/article/how-to-enable-speech-to-text-in-linux-with-this-simple-app/) (Speech Note 4.2)
- [Translator feature video demo on Sailfish OS](https://www.youtube.com/watch?v=88cdPpvBmmI) (Speech Note 4.0)
- [Translator feature video demo on PinePhone](https://www.youtube.com/watch?v=kTsM3kUxE2Q) (Speech Note 4.0)
- [DebugPoint.com](https://www.debugpoint.com/speech-note-text-to-speech/) (Speech Note 4.0)
- [DebugPoint.com video](https://youtu.be/dYIPyS3F_eU) (Speech Note 4.0)
- [OMG! Linux](https://www.omglinux.com/speech-note-transcribe-voice-to-text-on-linux/) (Speech Note 4.0)
- [LinuxLinks](https://www.linuxlinks.com/machine-learning-linux-speech-note/) (Speech Note 4.0)
- [The Linux Cast video](https://www.youtube.com/watch?v=zlLVgTB42Bo) (Speech Note 4.0)
- [CONNECTwww.com](https://connectwww.com/speech-note-offline-speech-to-text-text-to-speech-and-translation-app/) (Speech Note 4.0)

## License

**Speech Note** is an open source project. Source code is released under the
[Mozilla Public License Version 2.0](https://www.mozilla.org/MPL/2.0/).

3rd party libraries:

- **Coqui STT**, released under the
[Mozilla Public License Version 2.0](https://raw.githubusercontent.com/coqui-ai/STT/main/LICENSE)
- **Coqui TTS**, released under the
[Mozilla Public License Version 2.0](https://raw.githubusercontent.com/coqui-ai/TTS/dev/LICENSE.txt)
- **Vosk API**, released uder the [Apache License 2.0](https://raw.githubusercontent.com/alphacep/vosk-api/master/COPYING)
- **whisper.cpp**, released under the [MIT License](https://raw.githubusercontent.com/ggerganov/whisper.cpp/master/LICENSE)
- **WebRTC**, released under [this license](https://webrtc.googlesource.com/src/+/refs/heads/main/LICENSE)
- **libarchive**, released under the [BSD License](https://raw.githubusercontent.com/libarchive/libarchive/master/COPYING)
- **RNNoise-nu**, released under the [BSD 3-Clause License](https://raw.githubusercontent.com/GregorR/rnnoise-nu/master/COPYING)
- **{fmt}**, released uder [this license](https://raw.githubusercontent.com/fmtlib/fmt/master/LICENSE.rst)
- **Hugging Face Transformers**, released under the [Apache License 2.0](https://raw.githubusercontent.com/huggingface/transformers/main/LICENSE)
- **Piper**, released under the [MIT License](https://raw.githubusercontent.com/rhasspy/piper/master/LICENSE.md)
- **RHVoice**, released under the [GNU General Public License v2.0](https://raw.githubusercontent.com/RHVoice/RHVoice/master/LICENSE.md)
- **ssplit-cpp**, released under the [Apache License 2.0](https://github.com/ugermann/ssplit-cpp/raw/master/LICENSE.md)
- **espeak-ng**, released under the [GNU General Public License v3.0](https://raw.githubusercontent.com/espeak-ng/espeak-ng/master/COPYING)
- **bergamot-translator**, released under the [Mozilla Public License 2.0](https://raw.githubusercontent.com/browsermt/bergamot-translator/main/LICENSE)
- **Rubber Band Library**, released under the [GNU General Public License (version 2 or later)](https://breakfastquay.com/rubberband/license.html)
- **simdjson**, released under the [Apache License 2.0](https://github.com/simdjson/simdjson/raw/master/LICENSE)
- **Nlohmann JSON**, released under the [MIT License](https://json.nlohmann.me/home/license/)
- **uroman**, released under [this license](https://github.com/isi-nlp/uroman/raw/master/LICENSE.txt)
- **astrunc**, released under the [MIT License](https://raw.githubusercontent.com/Joke-Shi/astrunc/master/LICENSE)
- **FFmpeg**, released under the [GNU Lesser General Public License version 2.1 or later](https://git.ffmpeg.org/gitweb/ffmpeg.git/blob_plain/HEAD:/LICENSE.md)
- **LAME**, released under the LGPL
- **Vorbis**, released under [this license](https://gitlab.xiph.org/xiph/vorbis/-/raw/master/COPYING?ref_type=heads)
- **TagLib**, released under the [GNU Lesser General Public License (LGPL)](https://raw.githubusercontent.com/taglib/taglib/master/COPYING.LGPL)
              and [Mozilla Public License (MPL)](https://raw.githubusercontent.com/taglib/taglib/master/COPYING.MPL)
- **libnumbertext**, released under the [BSD License](https://raw.githubusercontent.com/Numbertext/libnumbertext/master/COPYING)
- **KDBusAddons**, released under the [LGPL licenses](https://invent.kde.org/frameworks/kdbusaddons/-/tree/master/LICENSES?ref_type=heads)
- **QHotkey**, released under the [BSD-3-Clause License](https://raw.githubusercontent.com/Skycoder42/QHotkey/master/LICENSE)
- **faster-whisper**, released under the [MIT License](https://github.com/guillaumekln/faster-whisper/raw/master/LICENSE)
- **Mimic 3**, released under the [AGPL-3.0 license](https://github.com/MycroftAI/mimic3/raw/master/LICENSE)
- **Unikud**, released under the [MIT License](https://github.com/morrisalp/unikud/raw/main/LICENSE)
- **april-asr**, released under the [GNU General Public License v3.0](https://github.com/abb128/april-asr/raw/main/COPYING)
- **libopus**, released under [this license](https://gitlab.xiph.org/xiph/opus/-/raw/master/COPYING)
- **html2md**, released under the [MIT License](https://opensource.org/licenses/MIT)
- **maddy**, released under the [MIT License](https://raw.githubusercontent.com/progsource/maddy/master/LICENSE)
- **WhisperSpeech**, released under the [MIT License](https://raw.githubusercontent.com/collabora/WhisperSpeech/main/LICENSE)
- **Kokoro**, released under the [Apache License 2.0](https://github.com/hexgrad/kokoro/raw/refs/heads/main/LICENSE)
- **Parler-TTS**, released under the [Apache License 2.0](https://raw.githubusercontent.com/huggingface/parler-tts/refs/heads/main/LICENSE)
- **F5-TTS**, released under the [MIT License](https://github.com/SWivid/F5-TTS/raw/refs/heads/main/LICENSE)

The files in the directory `nonbreaking_prefixes` were copied from
[mosesdecoder](https://github.com/moses-smt/mosesdecoder) project and distributed under the
[GNU Lesser General Public License v2.1](https://github.com/moses-smt/mosesdecoder/raw/master/COPYING).

