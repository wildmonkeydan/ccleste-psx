/** @file ngage_appview.h
 *
 *  A simple SDL 2.0.x sample project.
 *
 *  Do not modify this file unless absolutely necessary.
 *  All project-specific settings can be found in the file:
 *  project_config.cmake
 *
 *  To the extent possible under law, Michael Fitzmayer has waived all
 *  copyright and related or neighboring rights to SDLexample.  This
 *  work is published from: Germany.
 *
 *  CC0 http://creativecommons.org/publicdomain/zero/1.0/
 *  SPDX-License-Identifier: CC0-1.0
 *
 **/

#ifndef NGAGE_APPVIEW_H
#define NGAGE_APPVIEW_H

#include <coecntrl.h>

class CNGageAppView : public CCoeControl
{
public:
    static CNGageAppView* NewL(const TRect& aRect);
    static CNGageAppView* NewLC(const TRect& aRect);

    ~CNGageAppView();

public:
    void Draw(const TRect& aRect) const;

private:
    void ConstructL(const TRect& aRect);

    CNGageAppView();
};

#endif /* NGAGE_APPVIEW_H */
