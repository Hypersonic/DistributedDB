FleetBeetStreet
---------------

Query structure:
    - A full query structure BNF is included in `backend/parser/parser.h`
    - Queries are responded to with a list of newline-separated replies, followed by `DONE\n"
    - Multiple queries can be made in a single connection, but this is not currently used by the frontend

Debug messages can be turned on by defining the environment variable DEBUG before compiling the backend.

Currently, an excessive amount of queries are sent between the front and backend. This will be fixed soon(tm).

The backend is still heavily unoptimized, with O(n) lookups for basically everything. Indexing will soon be added to deal with this.

The backend schema is effectively baked into the application.

The backend does not currently save and load to/from files, so the storage only lives as long as the backend is running.

The frontend is hardcoded to connect to a host on `localhost:3001`. For now, this can be changed by editing `frontend/db.py`, in the `send_request` function.

The user interface on the frontend is still massively unfriendly.
