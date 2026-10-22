#pragma once

#include <QObject>
#include <QStringList>

class StreamingProfileManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList profileNames READ profileNames NOTIFY profilesChanged)

public:
    static StreamingProfileManager* get();

    QStringList profileNames() const;

    Q_INVOKABLE bool saveProfile(const QString& name);
    Q_INVOKABLE bool loadProfile(const QString& name);
    Q_INVOKABLE bool deleteProfile(const QString& name);
    Q_INVOKABLE bool hasProfile(const QString& name) const;

signals:
    void profilesChanged();
    void profileLoaded(const QString& name);
    void profileError(const QString& message);

private:
    explicit StreamingProfileManager(QObject* parent = nullptr);

    static bool isValidName(const QString& name);
};
