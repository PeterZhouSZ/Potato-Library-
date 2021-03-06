/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _MVGTK_PLOT_COMMAND_
#define _MVGTK_PLOT_COMMAND_


#include "mvgtk.h"


class PlotCommand : public MvgtkCommand
{
public:
    PlotCommand();

    virtual bool parseArgs(int argc, char** argv, int& argi);
    virtual bool execute(Mesh& mesh, const GlobalOptions* opts = 0);

    static const char* cmdOptions();
    static const char* cmdDesc();

private:
    unsigned m_layer;
    unsigned m_nSubdiv;
    std::string m_outFilename;
};


#endif
