/** @file ngage_document.cpp
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

#include "ngage_appui.h"
#include "ngage_document.h"

CNGageDocument* CNGageDocument::NewL(CEikApplication& aApp)
{
    CNGageDocument* self = NewLC(aApp);
    CleanupStack::Pop(self);
    return self;
}

CNGageDocument* CNGageDocument::NewLC(CEikApplication& aApp)
{
    CNGageDocument* self = new (ELeave) CNGageDocument(aApp);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CNGageDocument::ConstructL()
{
    /* No implementation required. */
}

CNGageDocument::CNGageDocument(CEikApplication& aApp) : CAknDocument(aApp)
{
    /* No implementation required. */
}

CNGageDocument::~CNGageDocument()
{
    /* No implementation required. */
}

CEikAppUi* CNGageDocument::CreateAppUiL()
{
    CEikAppUi* appUi = new (ELeave) CNGageAppUi;
    return appUi;
}
