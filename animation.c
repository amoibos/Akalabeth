#include "animation.h"
#include "animationdata.h"
#include "data.h"

sSpriteData get_sprite_data(signed short data) {
	sSpriteData retval = {{0, 0}, -1};

	retval.location.x = 	0x1F & (data >> 5);
	retval.location.y =  0x1F & data;
	retval.index = data >> 10;

	return retval;
}

void animate_quarterly(eScreens screen) {

	animation_frame = (animation_frame + 1) & 3;
	for(unsigned char pos=0; pos < sprites_no; ++pos)

		if (all_sprites[pos] != NO_MOTION) {
			sSpriteData sprite;
			sprite = get_sprite_data(all_sprites[pos]);

			switch(screen) {
				case ScreenTitle: {

					break;
				}
				case ScreenAppendix: case ScreenControls: case ScreenStory: case ScreenExplain: case ScreenExplain2: {
					print_font(sprite.location.x, sprite.location.y, PRESS_A_KEY[sprite.index][animation_frame]);
					break;
				}
				default:
					break;
			}

		}
		waitForVBlank();
}
