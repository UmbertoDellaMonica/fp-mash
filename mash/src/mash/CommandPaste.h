// Copyright © 2015, Battelle National Biodefense Institute (BNBI);
// all rights reserved. Authored by: Brian Ondov, Todd Treangen,
// Sergey Koren, and Adam Phillippy
//
// See the LICENSE.txt file included with this software for license information.

#ifndef INCLUDED_CommandPaste
#define INCLUDED_CommandPaste

#include "Command.h"
#include "Sketch.h"
#include "SketchFingerPrint.h"

namespace mash {

class CommandPaste : public Command
{
public:
    
    CommandPaste();
    
    int run() const; // override

    int runFingerPrint() const; 

private:

int checkArguments() const ;

std::string processArguments(std::vector<std::string>& files, bool output, bool list, bool fingerPrint) const;



};




} // namespace mash

#endif
