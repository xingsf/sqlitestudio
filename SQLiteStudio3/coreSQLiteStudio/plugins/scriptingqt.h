#ifndef SCRIPTINGQT_H
#define SCRIPTINGQT_H

#include "builtinplugin.h"
#include "scriptingplugin.h"
#include <QHash>
#include <QVariant>
#include <QCache>
#include <QJSValue>

class QMutex;
class ScriptingQtDbProxy;
class ScriptingQtDebugger;

class ScriptingQt : public BuiltInPlugin, public DbAwareScriptingPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN_TITLE("JavaScript scripting")
    SQLITESTUDIO_PLUGIN_DESC("JavaScript scripting support.")
    SQLITESTUDIO_PLUGIN_VERSION(10100)
    SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        ScriptingQt();
        ~ScriptingQt();

        static QJSValueList toValueList(QJSEngine* engine, const QList<QVariant>& values);

        QString getLanguage() const;
        Context* createContext();
        void releaseContext(Context* context);
        void resetContext(Context* context);
        QVariant evaluate(const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking = false, QString* errorMessage = nullptr);
        QVariant evaluate(Context* context, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking = false);
        void setVariable(Context* context, const QString& name, const QVariant& value);
        QVariant getVariable(Context* context, const QString& name);
        bool hasError(Context* context) const;
        QString getErrorMessage(Context* context) const;
        QString getIconPath() const;
        bool init();
        void deinit();

    private:
        using DbAwareScriptingPlugin::evaluate;

        class ContextQt : public ScriptingPlugin::Context
        {
            public:
                ContextQt();
                ~ContextQt();

                QJSEngine* engine = nullptr;
                QCache<QString, QJSValue> scriptCache;
                QString error;
                ScriptingQtDbProxy* dbProxy = nullptr;
                ScriptingQtDebugger* debugger = nullptr;
                QJSValue dbProxyScriptValue;
        };

        ContextQt* getContext(ScriptingPlugin::Context* context) const;
        QJSValue getFunctionValue(ContextQt* ctx, const QString& code, const FunctionInfo& funcInfo);
        QVariant evaluate(ContextQt* ctx, const QString& code, const FunctionInfo& funcInfo, const QList<QVariant>& args, Db* db, bool locking);
        QVariant convertVariant(const QVariant& value, bool wrapStrings = false);

        static const constexpr int cacheSize = 5;

        ContextQt* mainContext = nullptr;
        QList<Context*> contexts;
        QMutex* mainEngineMutex = nullptr;
};

class ScriptingQtDebugger : public QObject
{
    Q_OBJECT

    public:
        ScriptingQtDebugger(QJSEngine* engine);

    private:
        QJSEngine* engine = nullptr;

    public slots:
        QJSValue debug(const QVariant& value);
};

#endif // SCRIPTINGQT_H
