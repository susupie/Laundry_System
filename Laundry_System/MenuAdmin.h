#ifndef MENUADMIN_H
#define MENUADMIN_H

#include "db_conn.h"
#include <string>
#include <memory>

// Declaration of the Admin Menu function
void MenuAdmin(std::shared_ptr<DBConnection> db_conn, std::string adminName);

#endif