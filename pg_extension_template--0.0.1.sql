-- This program is open source, licensed under the PostgreSQL License.
-- For license terms, see the LICENSE file.
--
-- Copyright (c) 2023, FIXME

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_extension_template" to load this file. \quit

CREATE FUNCTION pg_extension_template(
    OUT ok boolean,
    OUT value text
)
RETURNS SETOF record
AS 'MODULE_PATHNAME', 'pg_extension_template'
LANGUAGE C STRICT VOLATILE;
