#ifndef MENUUSER_H
#define MENUUSER_H

#include "db_conn.h" // Needed for shared_ptr
#include <string>
#include <memory>

// Update arguments
void MenuUser(std::shared_ptr<DBConnection> db_conn, std::string userEmail);

#endif