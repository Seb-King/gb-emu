typedef  uint8_t   u8;
typedef   int8_t   s8;
typedef uint16_t  u16;

// --Registers--

	// Bit 7 - LCD Display Enable             (0=Off, 1=On)
	//  Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
	//  Bit 5 - Window Display Enable          (0=Off, 1=On)
	//  Bit 4 - BG & Window Tile Data Select   (0=8800-97FF, 1=8000-8FFF)
	//  Bit 3 - BG Tile Map Display Select     (0=9800-9BFF, 1=9C00-9FFF)
	//  Bit 2 - OBJ (Sprite) Size              (0=8x8, 1=8x16)
	//  Bit 1 - OBJ (Sprite) Display Enable    (0=Off, 1=On)
	//  Bit 0 - BG Display (for CGB see below) (0=Off, 1=On)

	// 0xFF40 - LCD Control 
	// Bit 7 - LCD Control Operation = 0; Stop completely (no pic on screen), = 1 : Operation
	// Bit 6 - Tilemap (OBJ) display select = 0 : 0x9800-0x9BFF, =1 : 0x9C00 0x9FFF
	// Bit 5 - Window display =0 : off, =1 : on
	// Bit 4 - BG and Window Tile Data select =0: 0x8800-0x97FF, =1 : 0x8000-0x8FFF (OBJ area)
	// Bit 3 - BG Tilemap display select =0: 0x9800-0x9BFF, =1 : 0x9C00-0x9FFF
	// Bit 2 - OBJ (Sprite) Size =0: 8x8, =1 : 8x16 (width * height)
	// Bit 1 - OBJ (Sprite) Display =0: off, =1 : on
	// Bit 0 - BG and Window display (same as above)
	u16 LCDC = 0xFF40;
	u16 STAT = 0xFF41;  // LCDC status (interrupts and memory access)
	u16 SCY  = 0xFF42; 	// - Scroll Y
	u16 SCX = 0xFF43; 	// - Scroll X
	u16 LY = 0xFF44;		//0xFF44 - LCDC Y-Coordinate (the vertical line to which present data is transferred to the LCD driver)
	u16 LYC = 0xFF45;		//0xFF45 - LY Compare : Compares itself with the LY. If the values ar ethe same, set the coincedent flag 
	// That is Bit 6 of STAT

	u16 DMA = 0xFF46; 	//0xFF46 - Direct Memory Access Transfer
	u16 BGP = 0xFF47; 	//0xFF47 - Background Palette Data  0b11 10 01 00
	// u16 OBP0, OBP1, WY, WX;