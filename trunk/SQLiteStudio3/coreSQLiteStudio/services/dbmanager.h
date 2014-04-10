#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "db/db.h"
#include "coreSQLiteStudio_global.h"
#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <QList>
#include <QHash>

/** @file */

class DbPlugin;
class Config;
class Plugin;
class PluginType;

/**
 * @brief Database registry manager.
 *
 * Manages list of databases in SQLiteStudio core.
 * Also keeps list of supported database types
 * (QSqlDriver names), like "QSQLITE", etc.
 *
 * It's a singleton asseccible with DBLIST macro.
 */
class API_EXPORT DbManager : public QObject
{
    Q_OBJECT

    public:
        /**
         * @brief Creates database manager.
         * @param parent Parent object passed to QObject constructor.
         */
        explicit DbManager(QObject *parent = 0);

        /**
         * @brief Default destructor.
         */
        ~DbManager();

        /**
         * @brief Adds database to the manager.
         * @param name Symbolic name of the database, as it will be presented in the application.
         * @param path Path to the database file.
         * @param options Key-value custom options for database, that can be used in the DbPlugin implementation, like connection password, etc.
         * @param permanent If true, then the database will be remembered in configuration, otherwise it will be disappear after application restart.
         * @return true if the database has been successfly added, or false otherwise.
         *
         * The method can return false if given database file exists, but is not supported SQLite version (including invalid files,
         * that are not SQLite database). It basicly returns false if DbPlugin#getInstance() returned null for given database parameters.
         */
        virtual bool addDb(const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent = true) = 0;

        /**
         * @overload bool addDb(const QString &name, const QString &path, bool permanent)
         */
        virtual bool addDb(const QString &name, const QString &path, bool permanent = true) = 0;

        /**
         * @brief Updates registered database with new data.
         * @param db Registered database.
         * @param name New symbolic name for the database.
         * @param path New database file path.
         * @param options New database options. See addDb() for details.
         * @param permanent True to make the database stored in configuration, false to make it disappear after application restart.
         * @return true if the database was successfly updated, or false otherwise.
         */
        virtual bool updateDb(Db* db, const QString &name, const QString &path, const QHash<QString, QVariant> &options, bool permanent) = 0;

        /**
         * @brief Removes database from application.
         * @param name Symbolic name of the database.
         * @param cs Should the name be compare with case sensitivity?
         */
        virtual void removeDbByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive) = 0;

        /**
         * @brief Removes database from application.
         * @param path Database file path as it was passed to addDb() or updateDb().
         */
        virtual void removeDbByPath(const QString& path) = 0;

        /**
         * @brief Removes database from application.
         * @param db Database to be removed.
         */
        virtual void removeDb(Db* db) = 0;

        /**
         * @brief Gives list of databases registered in the application.
         * @return List of databases, no matter if database is open or not.
         */
        virtual QList<Db*> getDbList() = 0;

        /**
         * @brief Gives list of currently open databases.
         * @return List of open databases.
         */
        virtual QList<Db*> getConnectedDbList() = 0;

        /**
         * @brief Gives list of database names.
         * @return List of database names that are registered in the application.
         */
        virtual QStringList getDbNames() = 0;

        /**
         * @brief Gives database object by its name.
         * @param name Symbolic name of the database.
         * @param cs Should the \p name be compared with case sensitivity?
         * @return Database object, or null pointer if the database could not be found.
         *
         * This method is fast, as it uses hash table lookup.
         */
        virtual Db* getByName(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive) = 0;

        /**
         * @brief Gives database object by its file path.
         * @param path Database file path as it was passed to addDb() or updateDb().
         * @return Database matched by file path, or null if no database was found.
         *
         * This method is fast, as it uses hash table lookup.
         */
        virtual Db* getByPath(const QString& path) = 0;

        /**
         * @brief Creates in-memory SQLite3 database.
         * @return Created database.
         *
         * Created database can be used for any purpose. Note that DbManager doesn't own created
         * database and it's up to the caller to delete the database when it's no longer needed.
         */
        virtual Db* createInMemDb() = 0;

        /**
         * @brief Generates database name.
         * @param filePath Database file path.
         * @return A name, using database file name as a hint for a name.
         *
         * This method doesn't care about uniqueness of the name. It just gets the file name from provided path
         * and uses it as a name.
         */
        static QString generateDbName(const QString& filePath);

    public slots:
        /**
         * @brief Tries to load all databases from configuration.
         *
         * Gets list of registered databases from configuration and for each of them
         * tries to find working DbPlugin. After the registered database was loaded successfully
         * by some DbPlugin, the database gets registered in DbManager and will later be
         * provided by getDbList(), getByName() and getByPath() methods.
         *
         * Any databases that failed to be loaded are not registered in DbManager.
         * To get full list of registered databases (even those not loaded), use Config::dbList().
         */
        virtual void loadDbListFromConfig() = 0;

    signals:
        /**
         * @brief Application just connected to the database.
         * @param db Database object that the connection was made to.
         *
         * Emitted just after application has connected to the database.
         */
        void dbConnected(Db* db);

        /**
         * @brief Application just disconnected from the database.
         * @param db Database object that the connection was closed with.
         */
        void dbDisconnected(Db* db);

        /**
         * @brief A database has been added to the application.
         * @param db Database added.
         * Emitted from addDb() methods in case of success.
         */
        void dbAdded(Db* db);

        /**
         * @brief A database has been removed from the application.
         * @param db Database object that was removed. The object still exists, but will be removed soon after this signal is handled.
         *
         * Emitted from removeDb(). As the argument is a smart pointer, the object will be deleted after last reference to the pointer
         * is deleted, which is very likely that the pointer instance in this signal is the last one.
         */
        void dbRemoved(Db* db);

        /**
         * @brief A database registration data has been updated.
         * @param oldName The name of the database before the update - in case the name was updated.
         * @param db Database object that was updated.
         *
         * Emitted from updateDb() after successful update.
         *
         * The name of the database is a key for tables related to the databases, so if it changed, we dbUpdated() provides
         * the original name before update, so any tables can be updated basing on the old name.
         */
        void dbUpdated(const QString& oldName, Db* db);

        /**
         * @brief Loaded plugin to support the database.
         * @param db Database object handled by the plugin.
         * @param plugin Plugin that handles the database.
         *
         * Emitted after a plugin was loaded and it turned out to handle the database that was already registered in the application,
         * but wasn't managed by database manager, because no handler plugin was loaded.
         */
        void dbLoaded(Db* db, DbPlugin* plugin);

        /**
         * @brief Plugin supporting the database is about to be unloaded.
         * @param db Database object to be removed from the manager.
         * @param plugin Plugin that handles the database.
         *
         * Emitted when PluginManager is about to unload the plugin which is handling the database.
         * All classes using this database object should stop using it immediately, or the application may crash.
         *
         * The plugin itself should not use this signal. Instead it should implement Plugin::deinit() method
         * to perform deinitialization before unloading. The Plugin::deinit() method is called before this signal is emitted.
         */
        void dbAboutToBeUnloaded(Db* db, DbPlugin* plugin);

        /**
         * @brief Emited when the initial database list has been loaded.
         */
        void dbListLoaded();
};

/**
 * @brief Database manager.
 * Provides direct access to the database manager.
 */
#define DBLIST SQLITESTUDIO->getDbManager()

#endif // DBMANAGER_H
