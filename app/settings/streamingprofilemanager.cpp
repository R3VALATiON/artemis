#include "streamingprofilemanager.h"
#include "streamingpreferences.h"

#include <QSettings>

namespace {
constexpr auto kProfilesGroup = "streamingProfiles";

void writeProfile(QSettings& settings, const StreamingPreferences* prefs)
{
    settings.setValue("width", prefs->width);
    settings.setValue("height", prefs->height);
    settings.setValue("fps", prefs->fps);
    settings.setValue("bitrateKbps", prefs->bitrateKbps);
    settings.setValue("unlockBitrate", prefs->unlockBitrate);
    settings.setValue("autoAdjustBitrate", prefs->autoAdjustBitrate);
    settings.setValue("enableVsync", prefs->enableVsync);
    settings.setValue("enableHdr", prefs->enableHdr);
    settings.setValue("enableYUV444", prefs->enableYUV444);
    settings.setValue("videoCodecConfig", static_cast<int>(prefs->videoCodecConfig));
    settings.setValue("videoDecoderSelection", static_cast<int>(prefs->videoDecoderSelection));
    settings.setValue("windowMode", static_cast<int>(prefs->windowMode));
    settings.setValue("absoluteMouseMode", prefs->absoluteMouseMode);
    settings.setValue("framePacing", prefs->framePacing);
    settings.setValue("useVirtualDisplay", prefs->useVirtualDisplay);
    settings.setValue("enableFractionalRefreshRate", prefs->enableFractionalRefreshRate);
    settings.setValue("customRefreshRate", prefs->customRefreshRate);
    settings.setValue("enableResolutionScaling", prefs->enableResolutionScaling);
    settings.setValue("resolutionScaleFactor", prefs->resolutionScaleFactor);
}

void readProfile(QSettings& settings, StreamingPreferences* prefs)
{
    prefs->width = settings.value("width", prefs->width).toInt();
    prefs->height = settings.value("height", prefs->height).toInt();
    prefs->fps = settings.value("fps", prefs->fps).toInt();
    prefs->bitrateKbps = settings.value("bitrateKbps", prefs->bitrateKbps).toInt();
    prefs->unlockBitrate = settings.value("unlockBitrate", prefs->unlockBitrate).toBool();
    prefs->autoAdjustBitrate = settings.value("autoAdjustBitrate", prefs->autoAdjustBitrate).toBool();
    prefs->enableVsync = settings.value("enableVsync", prefs->enableVsync).toBool();
    prefs->enableHdr = settings.value("enableHdr", prefs->enableHdr).toBool();
    prefs->enableYUV444 = settings.value("enableYUV444", prefs->enableYUV444).toBool();
    prefs->videoCodecConfig = static_cast<StreamingPreferences::VideoCodecConfig>(
        settings.value("videoCodecConfig", static_cast<int>(prefs->videoCodecConfig)).toInt());
    prefs->videoDecoderSelection = static_cast<StreamingPreferences::VideoDecoderSelection>(
        settings.value("videoDecoderSelection", static_cast<int>(prefs->videoDecoderSelection)).toInt());
    prefs->windowMode = static_cast<StreamingPreferences::WindowMode>(
        settings.value("windowMode", static_cast<int>(prefs->windowMode)).toInt());
    prefs->absoluteMouseMode = settings.value("absoluteMouseMode", prefs->absoluteMouseMode).toBool();
    prefs->framePacing = settings.value("framePacing", prefs->framePacing).toBool();
    prefs->useVirtualDisplay = settings.value("useVirtualDisplay", prefs->useVirtualDisplay).toBool();
    prefs->enableFractionalRefreshRate = settings.value(
        "enableFractionalRefreshRate", prefs->enableFractionalRefreshRate).toBool();
    prefs->customRefreshRate = settings.value("customRefreshRate", prefs->customRefreshRate).toDouble();
    prefs->enableResolutionScaling = settings.value(
        "enableResolutionScaling", prefs->enableResolutionScaling).toBool();
    prefs->resolutionScaleFactor = settings.value(
        "resolutionScaleFactor", prefs->resolutionScaleFactor).toInt();
}
}

StreamingProfileManager::StreamingProfileManager(QObject* parent)
    : QObject(parent)
{
}

StreamingProfileManager* StreamingProfileManager::get()
{
    static StreamingProfileManager instance;
    return &instance;
}

QStringList StreamingProfileManager::profileNames() const
{
    QSettings settings;
    settings.beginGroup(kProfilesGroup);
    const QStringList names = settings.childGroups();
    settings.endGroup();
    return names;
}

bool StreamingProfileManager::isValidName(const QString& name)
{
    return !name.trimmed().isEmpty() && name.trimmed().size() <= 64 && !name.contains('/');
}

bool StreamingProfileManager::saveProfile(const QString& name)
{
    const QString normalizedName = name.trimmed();
    if (!isValidName(normalizedName)) {
        emit profileError(tr("Profile names must be 1–64 characters and cannot contain '/'."));
        return false;
    }

    QSettings settings;
    settings.beginGroup(QStringLiteral("%1/%2").arg(kProfilesGroup, normalizedName));
    writeProfile(settings, StreamingPreferences::get());
    settings.endGroup();
    settings.sync();

    emit profilesChanged();
    return settings.status() == QSettings::NoError;
}

bool StreamingProfileManager::loadProfile(const QString& name)
{
    const QString normalizedName = name.trimmed();
    if (!hasProfile(normalizedName)) {
        emit profileError(tr("The selected streaming profile no longer exists."));
        return false;
    }

    QSettings settings;
    settings.beginGroup(QStringLiteral("%1/%2").arg(kProfilesGroup, normalizedName));
    StreamingPreferences* prefs = StreamingPreferences::get();
    readProfile(settings, prefs);
    settings.endGroup();
    prefs->save();

    emit profileLoaded(normalizedName);
    return true;
}

bool StreamingProfileManager::deleteProfile(const QString& name)
{
    const QString normalizedName = name.trimmed();
    if (!hasProfile(normalizedName)) {
        return false;
    }

    QSettings settings;
    settings.beginGroup(kProfilesGroup);
    settings.remove(normalizedName);
    settings.endGroup();
    settings.sync();

    emit profilesChanged();
    return settings.status() == QSettings::NoError;
}

bool StreamingProfileManager::hasProfile(const QString& name) const
{
    return profileNames().contains(name.trimmed());
}
