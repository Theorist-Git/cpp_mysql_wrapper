#include <stdlib.h>
#include <iostream>
#include <vector>
#include <optional>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "conn_macros.hpp"

void err_catch(sql::SQLException &e) {
    std::cout << RED <<  "- [ERROR]: SQLException in " << __FILE__;
    std::cout << RED << " (" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
    std::cout << RED << "- [ERROR]: " << e.what();
    std::cout << RED << " (MySQL error code: " << e.getErrorCode();
    std::cout << RED << ", SQLState: " << e.getSQLState() << " )" << RESET << std::endl;
}

sql::Connection* est_conn() {
    try {
        sql::Driver* driver;
        sql::Connection* conn;

        /* Create a connection */
        driver = get_driver_instance();
        conn = driver->connect(HOST, USER, KEY);

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: Connect(HOST, USER, KEY)" << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Connected to `" << USER << "` at `" << HOST << "`" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        return conn;
    }
    catch (sql::SQLException &e) {
        err_catch(e);
        exit(EXIT_FAILURE);
    }
}

int close_conn(sql::Connection* conn) {
    try {
        delete conn;
        
        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: delete conn" << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Disconnected from `" << USER << "` at `" << HOST << "`" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        return EXIT_SUCCESS;
    }
    catch (sql::SQLException &e) {
        err_catch(e);

        return EXIT_FAILURE;
    }
}

int create_db(std::string db, sql::Connection* conn) {
    try {
        sql::Statement *stmt;
        stmt = conn->createStatement();
        stmt->execute("CREATE DATABASE " + db);

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: CREATE DATABASE " << db << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Database `" << db << "` created successfully" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        delete stmt;

        return EXIT_SUCCESS;

    } catch(sql::SQLException &e) {
        err_catch(e);

        return EXIT_FAILURE;
    }
}

int delete_db(std::string db, sql::Connection* conn) {
    try {
        sql::Statement *stmt;
        stmt = conn->createStatement();
        stmt->execute("DROP DATABASE `" + db + "`");

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: DROP DATABASE " << db << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Database `" << db << "` Deleted successfully" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        delete stmt;

        return EXIT_SUCCESS;

    } catch(sql::SQLException &e) {
        err_catch(e);

        return EXIT_FAILURE;
    }
}

int create_table(std::string db, std::pair<std::string, std::string> table, sql::Connection* conn) {
    try {
        sql::Statement* stmt;
        stmt = conn->createStatement();
        conn->setSchema(db);
        stmt->execute("CREATE TABLE " + table.first + "(" + table.second + ")");

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: CREATE TABLE " + table.first + "(" + table.second + ")" << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Table `" << table.first << "` created in `" << db << "`" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        delete stmt;

        return EXIT_SUCCESS;

    } catch(sql::SQLException &e) {
        err_catch(e);

        return EXIT_FAILURE;
    }
}

sql::ResultSet* get_table_schema(
    std::string db, 
    std::pair<std::string, std::optional<std::string>> table,
    sql::Connection* conn
) {
    try {
        sql::Statement* stmt;
        sql::ResultSet* res;
        stmt = conn->createStatement();

        conn->setSchema(db);
        res = stmt->executeQuery("DESCRIBE " + table.first);

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: DESCRIBE TABLE" + table.first << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Scheme of `" << table.first << "` Returned" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        delete stmt;

        return res;

    } catch(sql::SQLException &e) {
        err_catch(e);

        return NULL;
    }
}

int delete_table(std::string db, std::pair<std::string, std::string> table, sql::Connection* conn) {
    try {
        sql::Statement* stmt;
        stmt = conn->createStatement();
        conn->setSchema(db);
        stmt->execute("DROP TABLE " + table.first);

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: DROP TABLE " + table.first << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Table `" << table.first << "` Deleted from `" << db << "`" << 
                RESET << std::endl;
                break;
            default:
                break;
        }

        delete stmt;

        return EXIT_SUCCESS;

    } catch(sql::SQLException &e) {
        err_catch(e);

        return EXIT_FAILURE;
    }
}

sql::ResultSet* exec_arbitrary_stmt(std::string db, sql::Connection* conn, std::string query) {
    try {
        sql::Statement* stmt;
        sql::ResultSet* res;
        stmt = conn->createStatement();
        conn->setSchema(db);
        
        res = stmt->executeQuery(query);

        switch (DEBUG_LEVEL) {
            case 0:
                std::cout << "- [DEBUG]: EXECUTED STATEMENT: \n\t`" << query << "`" << std::endl;
            case 1:
                std::cout << GREEN << "- [INFO] : Query successful `" << "Exit Code: " << EXIT_SUCCESS << 
                "\n\tResult returned by the function [an sql::ResultSet* type variable is returned]" << RESET << std::endl;
                break;
            default:
                break;
        }

        delete stmt;

        return res;

    } catch(sql::SQLException &e) {
        if (e.getErrorCode() == 0 && e.getSQLState() == "00000") {

            switch (DEBUG_LEVEL) {
                case 0:
                    std::cout << "- [DEBUG]: EXECUTED STATEMENT: \n\t`" << query << "`" << std::endl;
                case 1:
                    std::cout << GREEN << "- [INFO] : Query successful `" << "Exit Code: " << EXIT_SUCCESS << 
                    "\n\tQuery doesn't return a sql::ResultSet* type object" << RESET << std::endl;
                    break;
                default:
                    break;
            }

            return NULL;

        } else {
            err_catch(e);
            
            return NULL;
        }

    }
}
