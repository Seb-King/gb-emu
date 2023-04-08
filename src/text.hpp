#include <SDL2_ttf/SDL_ttf.h>
#include <string>

extern TTF_Font* font;

void loadFont();

void closeFont();

class Text {
public:
	Text();
	~Text();

	void free();

	bool loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* renderer);

	void render(int x, int y, SDL_Renderer* renderer, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL);

private:
	SDL_Texture* texture;

	int width;
	int height;
};
