/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "compactCommand.h"


bool CompactCommand::execute(Mesh& mesh, const GlobalOptions* opts)
{
    if(opts && opts->verbose) std::cout << "Compact...\n";

    // Mark unused nodes as deleted
    mesh.deleteUnusedNodes();

    // If verbose mode is on, print a summary of what is removed
    if(opts && opts->verbose)
    {
        int nv = mesh.verticesSize() - mesh.nVertices();
        int ne = mesh.edgesSize()    - mesh.nEdges();
        int nf = mesh.facesSize()    - mesh.nFaces();
        int nn = mesh.nodesSize()    - mesh.nNodes();
        if(nv) std::cout << "Removing " << nv << " vertices.\n";
        if(ne) std::cout << "Removing " << ne << " edges.\n";
        if(nf) std::cout << "Removing " << nf << " faces.\n";
        if(nn) std::cout << "Removing " << nn << " nodes.\n";
    }

    // Call garbage collection that effectively removes deleted elements and
    // compact internal arrays.
    mesh.garbageCollection();
    return true;
}


const char* CompactCommand::cmdOptions()
{
    return "";
}


const char* CompactCommand::cmdDesc()
{
    return "Remove unused nodes.";
}
