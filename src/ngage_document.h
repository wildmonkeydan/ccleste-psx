/** @file ngage_document.h
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

#ifndef NGAGE_DOCUMENT_H
#define NGAGE_DOCUMENT_H

#include <akndoc.h>

class CNGageAppUi;
class CEikApplication;

class CNGageDocument : public CAknDocument
{
public:
    static CNGageDocument* NewL(CEikApplication& aApp);
    static CNGageDocument* NewLC(CEikApplication& aApp);

    ~CNGageDocument();

public:
    CEikAppUi* CreateAppUiL();

private:
    void ConstructL();
    CNGageDocument(CEikApplication& aApp);
};

#endif /* NGAGE_DOCUMENT_H */
