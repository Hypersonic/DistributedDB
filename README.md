FleetBeetStreet
---------------

This is a masterless, quorumless distributed database, hardcoded for a specific task (a twitter clone).

The frontend assumes at least one member of the cluster is sitting on `localhost:3004`. You can change this by chainging the `known_hosts` array in `frontend/db.py` to point to your favorite DB host.

It chooses A and P over C, so try not to make conflicting queries :)

Some useful scripts for cluster manipulation are in `/scripts`

Query structure:
    - A full query structure BNF is included in `backend/parser/parser.h`
    - Queries are responded to with a list of newline-separated replies, followed by `DONE\n"
    - Multiple queries can be made in a single connection, but this is not currently used by the frontend

Debug messages can be turned on by defining the environment variable DEBUG before compiling the backend.


