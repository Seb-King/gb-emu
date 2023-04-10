#include "text.hpp"

TTF_Font* font = NULL;

void loadFont() {
	font = TTF_OpenFont("Cascadia.ttf", 16);
}

void closeFont() {
	TTF_CloseFont(font);
}

Text::Text() {
	texture = NULL;
	width = 0;
	height = 0;
}

Text::~Text() {
	free();
}

void Text::free() {
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
		width = 0;
		height = 0;
	}
}

bool Text::loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* renderer) {
	free();

	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if (textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	} else {
		texture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (texture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		} else {
			width = textSurface->w;
			height = textSurface->h;
		}


		SDL_FreeSurface(textSurface);
	}

	return textSurface != NULL;
}

void Text::render(int x, int y, SDL_Renderer* renderer, SDL_Rect* clip, double angle, SDL_Point* center) {
	SDL_Rect renderQuad = { x, y, width, height };

	if (clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(renderer, texture, clip, &renderQuad, angle, center, SDL_FLIP_NONE);
}