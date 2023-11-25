/*
 * Copyright (c) 2023 McEndu
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "config.h"
#include "quadustui.h"
#include "screen.h"
#include "widgets.h"
#include <quadus.h>
#include <stdbool.h>
#include <stdlib.h>

struct modeSelection
{
	int iMode;
	int iRule;
	int menuLevel;
};

#define LEVEL_MODE 0
#define LEVEL_RULESET 1

static const menuItem rulesets[] = {
	{ "Standard", &qdsRulesetStandard },
	{ "Arcade", &qdsRulesetArcade },
	{ NULL, NULL },
};

static const menuItem gamemodes[] = {
	{ "Marathon", &qdsModeMarathon },
	{ "Sprint", &qdsModeSprint },
	{ "Master", &qdsModeMaster },
	{ "Invisible Marathon", &qdsModeInvisible },
	{ NULL, NULL },
};

static const menuItem *menus[] = {
	[LEVEL_MODE] = gamemodes,
	[LEVEL_RULESET] = rulesets,
};

/* return false to stop menu from drawing */
static bool handleSelect(WINDOW *w, uiState *state, struct modeSelection *sel)
{
	if (state->input & INPUT_UI_CONFIRM) {
		if (sel->menuLevel == LEVEL_MODE) {
			werase(w);
			state->mode = gamemodes[sel->iMode].data;
			sel->menuLevel = LEVEL_RULESET;
		} else if (sel->menuLevel == LEVEL_RULESET) {
			state->ruleset = rulesets[sel->iRule].data;
			changeScreen(state, &screenGame);
			return false;
		}
	}

	return true;
}

static void screenEnter(WINDOW *w, uiState *state)
{
	werase(w);
	struct modeSelection *sel = state->screenData
		= malloc(sizeof(struct modeSelection));
	sel->iMode = 0;
	sel->iRule = 0;
	sel->menuLevel = LEVEL_MODE;
}

static void screenUpdate(WINDOW *w, uiState *state)
{
	static const rect menuarea = { 5, 4, 30, 19 };
	struct modeSelection *sel = state->screenData;

	int *i = 0;
	switch (sel->menuLevel) {
		case LEVEL_MODE:
			i = &sel->iMode;
			break;
		case LEVEL_RULESET:
			i = &sel->iRule;
			break;
	}

	*i = updateSelection(*i, menus[sel->menuLevel], state->input);
	if (!handleSelect(w, state, sel)) return;
	menu(w, &menuarea, menus[sel->menuLevel], *i);
}

static void screenExit(WINDOW *w, uiState *state)
{
	free(state->screenData);
}

const screen screenModeSelect = {
	.enter = screenEnter,
	.update = screenUpdate,
	.exit = screenExit,
};
