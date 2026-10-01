#pragma once

#include "Style.hpp"

struct Theme {
    Rgb path{145, 151, 166};
    Rgb folder{130, 177, 163};
    Rgb file{220, 224, 232};

    Rgb selec{54, 62, 80};

    Rgb lineNum{106, 113, 130};
    Rgb currLineNum{176, 195, 204};

    Rgb keyword{190, 156, 220};
    Rgb string{165, 198, 151};
    Rgb number{222, 176, 127};
    Rgb comment{115, 130, 125};
    Rgb function{143, 185, 215};

    Rgb matBracket{94, 112, 140};
    Rgb scopeGuide{70, 76, 91};
    Rgb separator{72, 78, 93};
};

inline Theme kiwiTheme;