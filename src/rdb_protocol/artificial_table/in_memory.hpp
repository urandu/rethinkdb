// Copyright 2010-2014 RethinkDB, all rights reserved.
#ifndef RDB_PROTOCOL_ARTIFICIAL_TABLE_IN_MEMORY_HPP_
#define RDB_PROTOCOL_ARTIFICIAL_TABLE_IN_MEMORY_HPP_

#include <map>
#include <string>
#include <vector>

#include "rdb_protocol/artificial_table/backend.hpp"
#include "rdb_protocol/datum.hpp"

/* This is the backend for an artificial table that acts as much as possible like a real
table. It accepts all reads and writes, storing the results in a `std::map`. It's used
for testing `artificial_table_t`. */

class in_memory_artificial_table_backend_t :
    public artificial_table_backend_t
{
public:
    std::string get_primary_key_name() {
        return "id";
    }

    bool read_all_primary_keys(
            signal_t *interruptor,
            std::vector<ql::datum_t> *keys_out,
            UNUSED std::string *error_out) {
        random_delay(interruptor);
        on_thread_t thread_switcher(home_thread());
        keys_out->clear();
        for (auto it = data.begin(); it != data.end(); ++it) {
            ql::datum_t key = it->second->get_field("id", ql::NOTHROW);
            guarantee(key.has());
            keys_out->push_back(key);
        }
        return true;
    }

    bool read_row(
            ql::datum_t primary_key,
            signal_t *interruptor,
            ql::datum_t *row_out,
            UNUSED std::string *error_out) {
        random_delay(interruptor);
        on_thread_t thread_switcher(home_thread());
        auto it = data.find(primary_key->print_primary());
        if (it != data.end()) {
            *row_out = it->second;
        } else {
            *row_out = ql::datum_t();
        }
        return true;
    }

    bool write_row(
            ql::datum_t primary_key,
            ql::datum_t new_value,
            signal_t *interruptor,
            UNUSED std::string *error_out) {
        random_delay(interruptor);
        on_thread_t thread_switcher(home_thread());
        if (new_value.has()) {
            data[primary_key->print_primary()] = new_value;
        } else {
            data.erase(primary_key->print_primary());
        }
        return true;
    }

private:
    /* The purpose of `random_delay()` is to mix things up a bit to increase the
    likelihood of exposing a bug in `artificial_table_t`. */
    void random_delay(signal_t *interruptor) {
        if (randint(2) == 0) {
            signal_timer_t timer;
            timer.start(randint(100));
            wait_interruptible(&timer, interruptor);
        }
    }

    std::map<std::string, ql::datum_t> data;
};

#endif /* RDB_PROTOCOL_ARTIFICIAL_TABLE_IN_MEMORY_HPP_ */
