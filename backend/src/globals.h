#pragma once

#include <memory>

#include "macros.h"
#include "postman/postman.h"
#include "storage/storage.h"

extern std::shared_ptr<postman::Postman> postmaster_general;
extern std::shared_ptr<storage::DB> mt_gox;
