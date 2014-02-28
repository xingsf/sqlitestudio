#include "clicommandsql.h"
#include "cli.h"
#include "parser/ast/sqliteselect.h"
#include "parser/parser.h"
#include "parser/parsererror.h"
#include "db/queryexecutor.h"
#include "db/sqlresults.h"
#include "qio.h"
#include "unused.h"
#include "cli_config.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlField>
#include <QList>

bool CliCommandSql::execute(QStringList args)
{
    Db* db = cli->getCurrentDb();
    if (!db || !db->isOpen())
    {
        println(tr("Database is not open."));
        return false;
    }

    quint32 maxLength = CFG_CLI.Console.ColumnMaxWidth.get();

    // Executor deletes itself later when called with lambda.
    QueryExecutor *executor = new QueryExecutor(db, args[0]);
    connect(executor, SIGNAL(executionFinished(SqlResultsPtr)), this, SIGNAL(execComplete()));
    connect(executor, SIGNAL(executionFailed(int,QString)), this, SLOT(executionFailed(int,QString)));
    connect(executor, SIGNAL(executionFailed(int,QString)), this, SIGNAL(execComplete()));

    executor->exec([=](SqlResultsPtr results)
    {
        if (results->isError())
            return; // should not happen, since results handler function is called only for successful executions

        // Columns
        foreach (const QueryExecutor::ResultColumnPtr& resCol, executor->getResultColumns())
            qOut << resCol->displayName.left(maxLength) << "|";

        qOut << "\n";

        // Data
        SqlResultsRowPtr row;
        while (!(row = results->next()).isNull())
        {
            foreach (QVariant value, row->valueList())
                qOut << value.toString().left(maxLength) << "|";

            qOut << "\n";
        }
        qOut.flush();
    });

    return true;
}

bool CliCommandSql::validate(QStringList args)
{
    if (args.size() != 1)
    {
        printUsage();
        return false;
    }

    if (!cli->getCurrentDb())
    {
        println(tr("No working database is set.\n"
                   "Call .use command to set working database.\n"
                   "Call .dblist to see list of all databases."));

        return false;
    }

    return true;
}

QString CliCommandSql::shortHelp() const
{
    return tr("executes SQL query");
}

QString CliCommandSql::fullHelp() const
{
    return tr(
                "This command is executed every time you enter SQL query in command prompt. "
                "It executes the query on the current working database (see help for .use for details). "
                "There's no sense in executing this command explicitly. Instead just type the SQL query "
                "in the command prompt, without any command prefixed."
             );
}

QString CliCommandSql::usage() const
{
    return "query "+tr("<sql>");
}

void CliCommandSql::executionFailed(int code, const QString& msg)
{
    UNUSED(code);
    qOut << tr("Query execution error: %1").arg(msg) << "\n\n";
    qOut.flush();
}
