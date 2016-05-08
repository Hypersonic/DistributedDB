FleetBeetStreet
---------------

This is a masterless, quorumless distributed database, hardcoded for a specific task (a twitter clone).

It chooses A and P over C, so try not to make conflicting queries :)

Some useful scripts for cluster manipulation are in `/scripts`

Query structure:
    - A full query structure BNF is included in `backend/parser/parser.h`
    - Queries are responded to with a list of newline-separated replies, followed by `DONE\n"
    - Multiple queries can be made in a single connection, but this is not currently used by the frontend

Debug messages can be turned on by defining the environment variable DEBUG before compiling the backend.


