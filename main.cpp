#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QVector>
#include "sqlite3.h"

static int processSelect(void *unused, int count, char **data, char **columns)
{
    int idx;

    qDebug() << "There are " << count << " column(s)\n";

    for (idx = 0; idx < count; idx++) {
        qDebug() << "The data in column \"" <<  columns[idx] << "\" is " <<  data[idx];
    }
    return 0;   //if return 1, select will break
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    sqlite3 *ppDb;
    sqlite3_open("test", &ppDb);    //open bd!!!
    if (ppDb == NULL) {
        qDebug() << "base is closed";
        return 1;
    }

    char *msgError;
    //execute insertion and creation request for sqlite
    int resReq = sqlite3_exec(ppDb, "create table if not exists test(id integer primary key, name varchar(32));", NULL, NULL, &msgError);
    //insert 1 records
    sqlite3_exec(ppDb, "insert into test(name) values (\"test1\");", NULL, NULL, &msgError);
    //insert multiple records
    sqlite3_exec(ppDb, "insert into test(name) values (\"test2\"),(\"test3\"),(\"test4\");", NULL, NULL, &msgError);
    //select * from .... with usage of callback function
    sqlite3_exec(ppDb, "select * from test;", processSelect, NULL, &msgError);

    //ooo lambda function to execute select request
    auto testCall { [](void *unused, int count, char **data, char **columns) -> int {
        Q_UNUSED(unused);
        int idx;
//            qDebug() << "There are " << count << " column(s)\n";
        for (idx = 0; idx < count; idx++) {
            qDebug() << "The data in column \"" <<  columns[idx] << "\" is " <<  data[idx];
        }
        return 0;
    }};
    sqlite3_exec(ppDb, "select * from test;", testCall, NULL, &msgError);

    //1. prepare sql request
    //2. next step to get data from table
    //3. finalize (free memory of *res)
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(ppDb, "SELECT * FROM test;", -1, &res, NULL);
    if (rc != SQLITE_OK) {
       qDebug() << "Failed to fetch data: " <<  sqlite3_errmsg(ppDb);
       sqlite3_close(ppDb);
    }

    while (sqlite3_step(res) == SQLITE_ROW) {
        int num_cols = sqlite3_column_count(res);
        const unsigned char* str;
        char* colName;
        int id;
        for (int i = 0; i < num_cols; i++) {
            colName = (char*)sqlite3_column_name(res, i);
            switch (sqlite3_column_type(res, i)) {
                case (SQLITE3_TEXT) :
                    str = sqlite3_column_text(res, i);
                    qDebug() << colName << " : " << (char*)str;
                break;
                case (SQLITE_INTEGER) :
                   id = sqlite3_column_int(res, i);
                   qDebug() << colName << " : " << id;
                break;
            }
        }

    }


    sqlite3_finalize(res);
//don't forget close the base data
    sqlite3_close(ppDb);

    return a.exec();
}
