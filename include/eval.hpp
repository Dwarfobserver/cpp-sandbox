
#pragma once

#include <string>


namespace sc {

    // TODO Separate in awaitable steps, why not :
    // 1. Create files
    // 2. Start compilation
    // 3. Wait compilation
    // 4. Clean owned files
    // 5. Start process
    // 6. Wait process
    // 7. Clean owned files + executable

    int eval(std::string const& code);

}
