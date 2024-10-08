#include "seagoo.h"

/* global db ptr for lex parser, does this put us on a list? */
// TODO there must be another better way to parse info into Yacc
sqlite3 * db = NULL;

#define CREATE_TABLES_SQL                    \
	"CREATE TABLE IF NOT EXISTS SourceFiles (" \
	"id INTEGER PRIMARY KEY AUTOINCREMENT,"    \
	"filepath TEXT NOT NULL UNIQUE);"

#define INSERT_SOURCEFILE_SQL \
	"INSERT OR IGNORE INTO SourceFiles (filepath) VALUES (?);"

// Define the Includes table
#define CREATE_INCLUDES_TABLE_SQL                            \
	"CREATE TABLE IF NOT EXISTS Includes ("                    \
	"id INTEGER PRIMARY KEY AUTOINCREMENT,"                    \
	"source_file_id INTEGER,"                                  \
	"included_file_id INTEGER,"                                \
	"FOREIGN KEY (source_file_id) REFERENCES SourceFiles(id)," \
	"FOREIGN KEY (included_file_id) REFERENCES SourceFiles(id));"

// Insert statement for the Includes table
#define INSERT_INCLUDE_SQL                                                    \
	"INSERT OR IGNORE INTO Includes (source_file_id, included_file_id) VALUES " \
	"(?, ?);"

// Query to lookup includes for a given source file
#define LOOKUP_INCLUDES_SQL                                              \
	"SELECT sf_included.filepath "                                         \
	"FROM Includes i "                                                     \
	"JOIN SourceFiles sf ON i.source_file_id = sf.id "                     \
	"JOIN SourceFiles sf_included ON i.included_file_id = sf_included.id " \
	"WHERE sf.filepath = ?;"

/* Initializes index database at a specific filepath with required tables */
int init_db(const char * db_filepath) {
	if (db_filepath == NULL) {
		log_error("Database filepath is NULL\n");
		return SQLITE_ERROR;
	}

	// sets the global database pointer
	printf("creating index at %s\n", db_filepath);
	int rc = sqlite3_open(db_filepath, &db);

	if (rc != SQLITE_OK) {
		log_error("Can't open database: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	rc = create_tables(db);
	if (rc != SQLITE_OK) {
		log_error("Failed to create tables: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return rc;
	}

	/* rc = sqlite3_exec(db, "VACUUM;", 0, 0, 0); */

	return SQLITE_OK;
}

/* Creates tables in the SQLite database */
// NOTE: this is perfectly fine for how, however we are getting #embed in the forseeable future
int create_tables(sqlite3 * db) {
	char * errmsg = 0;

	// Create SourceFiles table
	int rc = sqlite3_exec(db, CREATE_TABLES_SQL, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		log_error("SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return rc;
	}

	// Create Includes table
	rc = sqlite3_exec(db, CREATE_INCLUDES_TABLE_SQL, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		log_error("SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return rc;
	}

	return SQLITE_OK;
}

/* Offloads a SourceFileNode in-memory graph node into the database */
int insert_source_file(sqlite3 * db, const SourceFileNode * record) {
	sqlite3_stmt * stmt;
	int rc = sqlite3_prepare_v2(db, INSERT_SOURCEFILE_SQL, -1, &stmt, NULL);

	if (rc != SQLITE_OK) {
		log_error("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return rc;
	}

	sqlite3_bind_text(stmt, 1, record->filepath, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		log_error("Failed to insert data: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return rc;
}

/* Insert an included file into the database (called from Yacc) */
int insert_include(sqlite3 * db, int source_file_id, char * included_filepath) {
	sqlite3_stmt * stmt;
	int included_file_id;

	// Check if the included file already exists
	const char * check_sql = "SELECT id FROM SourceFiles WHERE filepath = ?";
	if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL) != SQLITE_OK) {
		log_error("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, included_filepath, -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		included_file_id = sqlite3_column_int(stmt, 0);
	} else {
		// If it doesn't exist, insert it
		const char * insert_sql = INSERT_SOURCEFILE_SQL;
		sqlite3_finalize(stmt);

		if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
			log_error("Failed to prepare insert statement: %s\n",
					sqlite3_errmsg(db));
			return -1;
		}

		sqlite3_bind_text(stmt, 1, included_filepath, -1, SQLITE_STATIC);
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			log_error("Failed to insert included file: %s\n",
					sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			return -1;
		}

		included_file_id = (int)sqlite3_last_insert_rowid(db);
	}

	sqlite3_finalize(stmt);

	// Now insert the relationship into the Includes table
	const char * insert_include_sql = INSERT_INCLUDE_SQL;
	if (sqlite3_prepare_v2(db, insert_include_sql, -1, &stmt, NULL) != SQLITE_OK) {
		log_error("Failed to prepare include statement: %s\n",
				sqlite3_errmsg(db));
		return -1;
	}

	sqlite3_bind_int(stmt, 1, source_file_id);
	sqlite3_bind_int(stmt, 2, included_file_id);

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		log_error("Failed to insert include relationship: %s\n",
				sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return 0;
}

/* Lookup included files for a given source file */
includes_vector_t * lookup_includes(sqlite3 * db, const char * filepath) {
	sqlite3_stmt * stmt;
	const char * sql = LOOKUP_INCLUDES_SQL;

	includes_vector_t * result = malloc(sizeof(includes_vector_t));
	if (!result) {
		log_error("Memory allocation failed\n");
		return NULL;
	}
	kv_init(*result);

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		log_error("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		free(result);
		return NULL;
	}

	sqlite3_bind_text(stmt, 1, filepath, -1, SQLITE_STATIC);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char * included_file = (const char *)sqlite3_column_text(stmt, 0);
		kv_push(char *, *result, strdup(included_file));
	}
	sqlite3_finalize(stmt);

	return result;
}

/* Function to get the source file ID from the SourceFiles table */
int get_source_file_id(sqlite3 * db, char * filepath) {
	sqlite3_stmt * stmt;
	const char * sql = "SELECT id FROM SourceFiles WHERE filepath = ?";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		log_error("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, filepath, -1, SQLITE_STATIC);

	int source_file_id = -1;  // default to -1 if not found
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		source_file_id = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	return source_file_id;  // return found ID or -1 if not found
}

/* Closes the database */
int close_db(sqlite3 * db) {
	if (db)
		return sqlite3_close(db);
	return -1;
}

