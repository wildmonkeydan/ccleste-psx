#include <psxapi.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxspu.h>
#include <psxcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>

#include "celeste.h"
#include "tilemap.h"

static unsigned char ramAddr[1000000];

#define PICO8_W 128
#define PICO8_H 128

// OT and Packet Buffer sizes
#define OT_LEN			4094
#define PACKET_LEN		20768

// Screen resolution
#define SCREEN_XRES		320
#define SCREEN_YRES		240
#define SCREEN_XOFFSET  96
#define SCREEN_YOFFSET  56

#define CD_SECTOR_SIZE  2048
#define BtoS(len) ( ( len + CD_SECTOR_SIZE - 1 ) / CD_SECTOR_SIZE )



//
// SPU Vars
//


#define SPU_00CH (0x1L<< 0)
#define SPU_01CH (0x1L<< 1)
#define SPU_02CH (0x1L<< 2)
#define SPU_03CH (0x1L<< 3)
#define SPU_04CH (0x1L<< 4)
#define SPU_05CH (0x1L<< 5)
#define SPU_06CH (0x1L<< 6)
#define SPU_07CH (0x1L<< 7)
#define SPU_08CH (0x1L<< 8)
#define SPU_09CH (0x1L<< 9)
#define SPU_10CH (0x1L<<10)
#define SPU_11CH (0x1L<<11)
#define SPU_12CH (0x1L<<12)
#define SPU_13CH (0x1L<<13)
#define SPU_14CH (0x1L<<14)
#define SPU_15CH (0x1L<<15)
#define SPU_16CH (0x1L<<16)
#define SPU_17CH (0x1L<<17)
#define SPU_18CH (0x1L<<18)
#define SPU_19CH (0x1L<<19)

#define SPU_20CH (0x1L<<20)
#define SPU_21CH (0x1L<<21)
#define SPU_22CH (0x1L<<22)
#define SPU_23CH (0x1L<<23)

#define SPU_0CH SPU_00CH
#define SPU_1CH SPU_01CH
#define SPU_2CH SPU_02CH
#define SPU_3CH SPU_03CH
#define SPU_4CH SPU_04CH
#define SPU_5CH SPU_05CH
#define SPU_6CH SPU_06CH
#define SPU_7CH SPU_07CH
#define SPU_8CH SPU_08CH
#define SPU_9CH SPU_09CH


#define MALLOC_MAX 3            // Max number of time we can call SpuMalloc
//~ // convert Little endian to Big endian
#define SWAP_ENDIAN32(x) (((x)>>24) | (((x)>>8) & 0xFF00) | (((x)<<8) & 0x00FF0000) | ((x)<<24))
typedef struct VAGheader {       // All the values in this header must be big endian
	char id[4];             // VAGp         4 bytes -> 1 char * 4
	unsigned int version;          // 4 bytes
	unsigned int reserved;         // 4 bytes
	unsigned int dataSize;         // (in bytes) 4 bytes
	unsigned int samplingFrequency;// 4 bytes
	char  reserved2[12];    // 12 bytes -> 1 char * 12
	char  name[16];         // 16 bytes -> 1 char * 16
	// Waveform data after that
}VAGhdr;
u_long vag_spu_address;                  // address allocated in memory for first sound file
// DEBUG : these allow printing values for debugging
u_long spu_start_address;
u_long get_start_addr;
u_long transSize;

CdlLOC loc[14];
int ntoc;

static int next_channel = 0;
static int next_sample_addr = 0x1010;






//
// GPU Vars
//

// Double buffer structure
typedef struct {
	DISPENV	disp;			// Display environment
	DRAWENV	draw;			// Drawing environment
	u_long 	ot[OT_LEN];		// Ordering table
	char 	p[PACKET_LEN];	// Packet buffer
} DB;

// Double buffer variables
static DB	db[2];
static int	db_active = 0;
static char* db_nextpri;

static RECT	screen_clip;




//
// Controller Vars
//

typedef enum {
	// Standard pads, analog joystick, Jogcon
	PAD_SELECT = 1 << 0,
	PAD_L3 = 1 << 1,
	PAD_R3 = 1 << 2,
	PAD_START = 1 << 3,
	PAD_UP = 1 << 4,
	PAD_RIGHT = 1 << 5,
	PAD_DOWN = 1 << 6,
	PAD_LEFT = 1 << 7,
	PAD_L2 = 1 << 8,
	PAD_R2 = 1 << 9,
	PAD_L1 = 1 << 10,
	PAD_R1 = 1 << 11,
	PAD_TRIANGLE = 1 << 12,
	PAD_CIRCLE = 1 << 13,
	PAD_CROSS = 1 << 14,
	PAD_SQUARE = 1 << 15,

	// Mouse
	MOUSE_LEFT = 1 << 10,
	MOUSE_RIGHT = 1 << 11,

	// neGcon
	NCON_START = 1 << 3,
	NCON_UP = 1 << 4,
	NCON_RIGHT = 1 << 5,
	NCON_DOWN = 1 << 6,
	NCON_LEFT = 1 << 7,
	NCON_R = 1 << 8,
	NCON_B = 1 << 9,
	NCON_A = 1 << 10,

	// Guncon
	GCON_A = 1 << 3,
	GCON_TRIGGER = 1 << 13,
	GCON_B = 1 << 14
} PadButton;

typedef struct _PADTYPE
{
	unsigned char	stat;
	unsigned char	len : 4;
	unsigned char	type : 4;
	unsigned short	btn;
	unsigned char	rs_x, rs_y;
	unsigned char	ls_x, ls_y;
} PADTYPE;

PADTYPE* pad;

// Pad data buffer
uint8_t pad_buff[2][34];








static CVECTOR basePalette[16] = {
    { 0x00, 0x00, 0x00, 0 },
    { 0x1d, 0x2b, 0x53, 0 },
    { 0x7e, 0x25, 0x53, 0 },
    { 0x00, 0x87, 0x51, 0 },
    { 0xab, 0x52, 0x36, 0 },
    { 0x5f, 0x57, 0x4f, 0 },
    { 0xc2, 0xc3, 0xc7, 0 },
    { 0xff, 0xf1, 0xe8, 0 },
    { 0xff, 0x00, 0x4d, 0 },
    { 0xff, 0xa3, 0x00, 0 },
    { 0xff, 0xec, 0x27, 0 },
    { 0x00, 0xe4, 0x36, 0 },
    { 0x29, 0xad, 0xff, 0 },
    { 0x83, 0x76, 0x9c, 0 },
    { 0xff, 0x77, 0xa8, 0 },
    { 0xff, 0xcc, 0xaa, 0 }
};

static CVECTOR palette[16];

static void* initial_game_state = NULL;

static bool paused = false;

static int currentMusic = 2;
static int mus[6] = { 2,3,4,5,6,-1 };

static uint16_t buttonState = 0;


u_char currentTPage = 0;



void mainLoop();
static void p8_rectfill(int x0, int y0, int x1, int y1, int col);
static void p8_print(const char* str, int x, int y, int col);

void init() {

	// Reset the GPU, also installs a VSync event handler
	ResetGraph(0);

	// Set display and draw environment areas
	// (display and draw areas must be separate, otherwise hello flicker)
	SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&db[0].draw, 0, 256, SCREEN_XRES, SCREEN_YRES);

	// Enable draw area clear and dither processing
	setRGB0(&db[0].draw, 0, 0, 0);
	db[0].draw.isbg = 1;
	db[0].draw.dtd = 1;


	// Define the second set of display/draw environments
	SetDefDispEnv(&db[1].disp, 0, 256, SCREEN_XRES, SCREEN_YRES);
	SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_XRES, SCREEN_YRES);

	setRGB0(&db[1].draw, 0, 0, 0);
	db[1].draw.isbg = 1;
	db[1].draw.dtd = 1;

	SetDispMask(1);

	// Apply the drawing environment of the first double buffer
	PutDispEnv(&db[0].disp);
	PutDrawEnv(&db[0].draw);

	// Clear both ordering tables to make sure they are clean at the start
	ClearOTagR((uint32_t*)db[0].ot, OT_LEN);
	ClearOTagR((uint32_t*)db[1].ot, OT_LEN);

	// Set primitive pointer address
	db_nextpri = db[0].p;

	// Set clip region
	setRECT(&screen_clip, 0, 0, SCREEN_XRES, SCREEN_YRES);

	CdInit();

	EnterCriticalSection();
	InitHeap((u_long*)ramAddr, sizeof(ramAddr));
	ExitCriticalSection();

	SpuInit();


	// Master volume should be in range 0x0000 - 0x3fff
	SpuSetCommonMasterVolume(0x3fff, 0x3fff);
	// Cd volume should be in range 0x0000 - 0x7fff
	SpuSetCommonCDVolume(0x2fff, 0x2fff);
	// Set transfer mode 
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);

	// Init BIOS pad driver and set pad buffers (buffers are updated
	// automatically on every V-Blank)
	InitPAD(&pad_buff[0][0], 34, &pad_buff[1][0], 34);

	// Start pad
	StartPAD();

	// Don't make pad driver acknowledge V-Blank IRQ (recommended)
	ChangeClearPAD(0);

	FntLoad(960, 0);
	FntOpen(108, 220, 200, 16, 0, 200);
}

bool resetButtons() {
	if (pad->stat == 0) {
		// For digital pad, dual-analog and dual-shock
		if ((pad->type == 0x4) || (pad->type == 0x5) || (pad->type == 0x7)) {
			if (!(pad->btn & PAD_SELECT) && !(pad->btn & PAD_START) && !(pad->btn & PAD_TRIANGLE)) {
				return true;
			}
		}
	}
	return false;
}

u_long sendVAGtoRAM(unsigned int VAG_data_size, unsigned char* VAG_data) {
	
	int _addr = next_sample_addr;
	int _size = (VAG_data_size + 63) & 0xffffffc0;

    u_long size;
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);                              // DMA transfer; can do other processing during transfer
	SpuSetTransferStartAddr(_addr);

    size = SpuWrite((uint32_t*)VAG_data + sizeof(VAGhdr), _size);     // transfer VAG_data_size bytes from VAG_data  address to sound buffer
    SpuIsTransferCompleted(SPU_TRANSFER_WAIT);                     // Checks whether transfer is completed and waits for completion

	next_sample_addr = _addr + _size;
    return size;
}

void setVoiceAttr(unsigned int pitch, long channel, unsigned long soundAddr) {
	SpuSetVoiceVolume(channel, 0x2000, 0x2000);
	SpuSetVoicePitch(channel, pitch);						//~ Interval (set pitch)
	SpuSetVoiceStartAddr(channel, soundAddr);               //~ Waveform data start address
	SpuSetVoiceADSR(channel, 0x0, 0x0, 0x0, 0x0, 0xf);
}

void spu_playSFX(unsigned long channel) {
    SpuSetKey(1, channel);                               // Set several channels by ORing  each channel bit ; ex : SpuSetKey(SpuOn,SPU_0CH | SPU_3CH | SPU_8CH); channels 0, 3, 8 are on.
}

void spu_LoadVAG(u_long* data, unsigned long channel) {
    const VAGhdr* VAGfileHeader = (VAGhdr*)data;
    unsigned int pitch = (SWAP_ENDIAN32(VAGfileHeader->samplingFrequency) << 12) / 44100L;
    spu_start_address = SpuSetTransferStartAddr(vag_spu_address);                         // Sets a starting address in the sound buffer
    get_start_addr = SpuGetTransferStartAddr();                                        // SpuGetTransferStartAddr() returns current sound buffer transfer start address.
    transSize = sendVAGtoRAM(SWAP_ENDIAN32(VAGfileHeader->dataSize), (u_char*)data);
    setVoiceAttr(pitch, channel, vag_spu_address);
}

void cdMusic_Ready() {
	u_char result;
	uint8_t cmd = CdlModeDA | CdlModeRept;
	while ((ntoc = CdGetToc(loc)) == 0) { 		/* Read TOC */
		printf("No TOC found: please use CD-DA disc...\n");
	}

	for (int i = 1; i < ntoc; i++) {
		CdIntToPos(CdPosToInt(&loc[i]) - 74, &loc[i]);
		printf("Track %d  min %d sec %d sector %d\n", loc[i].track, loc[i].minute, loc[i].second, loc[i].sector);
	}
	CdControlB(CdlSetmode, &cmd, &result);
	VSync(3);
	printf("Result: %d", (int)ntoc);
}

uint32_t* LoadFile(const char* filename) {
	CdlFILE	file;
	uint32_t* buffer;


	printf("\nReading file %s... ", filename);

	// Search for the file
	if (!CdSearchFile(&file, filename))
	{
		// Return value is NULL, file is not found
		printf("Not found!\n");
		buffer = NULL;
		//return NULL;
	}

	// Allocate a buffer for the file
	buffer = (uint32_t*)malloc(2048 * ((file.size + 2047) / 2048));

	// Set seek target (seek actually happens on CdRead())
	CdControl(CdlSetloc, (unsigned char*)&file.pos, 0);

	// Read sectors
	CdRead((int)BtoS(file.size), (uint32_t*)buffer, CdlModeSpeed);

	// Wait until read has completed
	CdReadSync(0, 0);

	printf("Done.\n");

	return buffer;
}

void LoadTexture(const char* filename) {

	TIM_IMAGE tim = { 0 };
	uint32_t* tex = LoadFile(filename);

	GetTimInfo(tex, &tim);

	// Upload pixel data to framebuffer
	LoadImage(tim.prect, tim.paddr);
	DrawSync(0);

	// Upload CLUT to framebuffer
	if (tim.mode & 0x8) {
		LoadImage(tim.crect, tim.caddr);
		DrawSync(0);
	}

	//free(tex);
}

void LoadSound(const char* filename, unsigned long channel) {
	uint32_t* data = LoadFile(filename);

	spu_LoadVAG((u_long*)data, channel);

	free(data);
}

void LoadData() {

	LoadTexture("\\GFX.TIM;1");
	LoadTexture("\\FONT.TIM;1");

	/*LoadSound("\\SND0.VAG;1", SPU_01CH);
	LoadSound("\\SND1.VAG;1", SPU_02CH);
	LoadSound("\\SND2.VAG;1", SPU_03CH);
	LoadSound("\\SND3.VAG;1", SPU_04CH);
	LoadSound("\\SND4.VAG;1", SPU_05CH);
	LoadSound("\\SND5.VAG;1", SPU_06CH);
	LoadSound("\\SND6.VAG;1", SPU_07CH);
	LoadSound("\\SND7.VAG;1", SPU_08CH);
	LoadSound("\\SND8.VAG;1", SPU_09CH);
	LoadSound("\\SND9.VAG;1", SPU_10CH);
	LoadSound("\\SND13.VAG;1", SPU_11CH);
	LoadSound("\\SND14.VAG;1", SPU_12CH);
	LoadSound("\\SND15.VAG;1", SPU_13CH);
	LoadSound("\\SND16.VAG;1", SPU_14CH);
	LoadSound("\\SND23.VAG;1", SPU_15CH);
	LoadSound("\\SND35.VAG;1", SPU_16CH);
	LoadSound("\\SND37.VAG;1", SPU_17CH);
	LoadSound("\\SND38.VAG;1", SPU_18CH);
	LoadSound("\\SND40.VAG;1", SPU_19CH);
	LoadSound("\\SND50.VAG;1", SPU_20CH);
	LoadSound("\\SND51.VAG;1", SPU_21CH);
	LoadSound("\\SND54.VAG;1", SPU_22CH);
	LoadSound("\\SND55.VAG;1", SPU_23CH);*/

	printf("Assets Loaded\n");
}

void main(void) {
	
	init();
	printf("CELESTE CLASSIC PSX\n");
	int pico8emu(CELESTE_P8_CALLBACK_TYPE call, ...);
	

	LoadData();
	cdMusic_Ready();

	printf("Music Ready\n");

	// Set to the default palette
	memcpy(&palette, &basePalette, 64);

	Celeste_P8_set_call_func(pico8emu);

	printf("pico8emu setup    state size = %d\n", Celeste_P8_get_state_size());

	initial_game_state = malloc(Celeste_P8_get_state_size());
	if (initial_game_state) {
		Celeste_P8_save_state(initial_game_state);
	}

	Celeste_P8_set_rndseed(rand());

	Celeste_P8_init();

	printf("initial save sate done!\n");

	while (1) {
		mainLoop();
		//printf("loop\n");
	}
}

void mainLoop() {
	static int resetInputTimer = 0;
	uint16_t prevButtons = buttonState;
	
	pad = (PADTYPE*)&pad_buff[0][0];

	//hold select+start+triangle to reset
	if (initial_game_state != NULL && resetButtons()) {
		resetInputTimer++;
		if (resetInputTimer >= 30)
		{
			resetInputTimer = 0;
			paused = false;
			Celeste_P8_load_state(initial_game_state);
			Celeste_P8_set_rndseed(rand());

			Celeste_P8_init();
		}
	}
	else {
		resetInputTimer = 0;
	}

	prevButtons = buttonState;
	buttonState = 0;

	if (paused)
	{
		const int x0 = PICO8_W / 2 - 3 * 4, y0 = 8;

		p8_rectfill(x0 - 1, y0 - 1, 6 * 4 + x0 + 1, 6 + y0 + 1, 6);
		p8_rectfill(x0, y0, 6 * 4 + x0, 6 + y0, 0);
		p8_print("paused", x0 + 1, y0 + 1, 7);
	}
	else
	{

		Celeste_P8_update();
		currentTPage = 1;
		Celeste_P8_draw();
	}

	// Wait for GPU to finish drawing and vertical retrace
	DrawSync(0);
	VSync(0);
	DrawSync(0);
	VSync(0);

	// Swap buffers
	db_active ^= 1;
	db_nextpri = db[db_active].p;

	// Clear the OT of the next frame
	if (!paused) {
		ClearOTagR((uint32_t*)db[db_active].ot, OT_LEN);
	}

	// Apply display/drawing environments
	PutDrawEnv(&db[db_active].draw);
	PutDispEnv(&db[db_active].disp);

	// Enable display
	SetDispMask(1);

	// Start drawing the OT of the last buffer
	DrawOTag((uint32_t*)db[1 - db_active].ot + (OT_LEN - 1));
}

static void p8_rectfill(int x0, int y0, int x1, int y1, int col)
{
	int w = (x1 - x0 + 1);
	int h = (y1 - y0 + 1);
	if (w > 0 && h > 0)
	{
		TILE* tile = (TILE*)db_nextpri;

		setTile(tile);
		setWH(tile, w, h);
		setXY0(tile, x0 + SCREEN_XOFFSET, y0 + SCREEN_YOFFSET);
		setRGB0(tile, basePalette[col % 16].r, basePalette[col % 16].g, basePalette[col % 16].b);

		addPrim(&db[db_active].ot, tile);
		tile++;
		db_nextpri += sizeof(TILE);
	}
}

static void p8_print(const char* str, int x, int y, int col)
{
	SPRT_8* sprt;
	char c;
	for (c = *str; c; c = *(++str))
	{
		RECT srcrc;
		c &= 0x7F;
		srcrc.x = 8 * (c % 16);
		srcrc.y = 8 * (c / 16);
		srcrc.w = srcrc.h = 8;

		sprt = (SPRT_8*)db_nextpri;

		setSprt8(sprt);
		setXY0(sprt, x + SCREEN_XOFFSET, y + SCREEN_YOFFSET);
		setUV0(sprt, srcrc.x, srcrc.y);
		setClut(sprt, 640, 480);
		setRGB0(sprt, 128, 128, 128);

		addPrim(&db[db_active].ot, sprt);
		sprt++;
		db_nextpri += sizeof(SPRT_8);
		
		x += 4;
	}

	// Set Font T-Page

	DR_TPAGE* tprit = (DR_TPAGE*)db_nextpri;

	setDrawTPage(tprit, 0, 1, getTPage(0 & 0x3, 0, 640, 256));
	addPrim(&db[db_active].ot, tprit);
	tprit++;

	db_nextpri += sizeof(DR_TPAGE);
}

static bool inputPoll(int btn) {
	if (pad->stat == 0) {
		if ((pad->type == 0x4) || (pad->type == 0x5) || (pad->type == 0x7)) {

			switch (btn) {
			case k_left:
				if (!(pad->btn & PAD_LEFT))  return true;
				if ((pad->type == 0x5) || (pad->type == 0x7)) {
					if ((pad->ls_x - 128) < -64) {
						return true;
					}
				}
				break;
			case k_right:
				if (!(pad->btn & PAD_RIGHT)) return true;
				if ((pad->type == 0x5) || (pad->type == 0x7)) {
					if ((pad->ls_x - 128) > 64) {
						return true;
					}
				}
				break;
			case k_up:
				if (!(pad->btn & PAD_UP))    return true;
				if ((pad->type == 0x5) || (pad->type == 0x7)) {
					if ((pad->ls_y - 128) < -64) {
						return true;
					}
				}
				break;
			case k_down:
				if (!(pad->btn & PAD_DOWN))  return true;
				if ((pad->type == 0x5) || (pad->type == 0x7)) {
					if ((pad->ls_y - 128) > 64) {
						return true;
					}
				}
				break;
			case k_jump:
				if (!(pad->btn & PAD_CROSS)) return true;
				break;
			case k_dash:
				if (!(pad->btn & PAD_CIRCLE)) return true;
				break;



			}
			if (!(pad->btn & PAD_START)) {

					//Music Stuff

					paused = !paused;
			}
		}
	}
	return false;
}

static int gettileflag(int tile, int flag)
{
	return tile < sizeof(tile_flags) / sizeof(*tile_flags) && (tile_flags[tile] & (1 << flag)) != 0;
}

unsigned long soundId2SPUChannel(int id) {
	switch (id) {
	case 0:
		return SPU_01CH;
		break;
	case 1:
		return SPU_02CH;
		break;
	case 2:
		return SPU_03CH;
		break;
	case 3:
		return SPU_04CH;
		break;
	case 4:
		return SPU_05CH;
		break;
	case 5:
		return SPU_06CH;
		break;
	case 6:
		return SPU_07CH;
		break;
	case 7:
		return SPU_08CH;
		break;
	case 8:
		return SPU_09CH;
		break;
	case 9:
		return SPU_10CH;
		break;
	case 10:
		return SPU_11CH;
		break;
	case 11:
		return SPU_12CH;
		break;
	case 12:
		return SPU_13CH;
		break;
	case 13:
		return SPU_14CH;
		break;
	case 14:
		return SPU_15CH;
		break;
	case 15:
		return SPU_16CH;
		break;
	case 16:
		return SPU_17CH;
		break;
	case 17:
		return SPU_18CH;
		break;
	case 18:
		return SPU_19CH;
		break;
	case 19:
		return SPU_20CH;
		break;
	case 20:
		return SPU_21CH;
		break;
	case 21:
		return SPU_22CH;
		break;
	case 22:
		return SPU_23CH;
		break;
	default:
		return SPU_00CH;
		break;
	}
}

int pico8emu(CELESTE_P8_CALLBACK_TYPE call, ...) {
	static int camera_x = 0, camera_y = 0;
	va_list args;
	int ret = 0;
	SPRT_8* sprt;
	DR_TPAGE* tprit;
	LINE_F2* line;
	CdlLOC track;
	int b = 0;
	int col;
	int x, y;
	int tx;
	int x0, y0, x1, y1;
	int mask;

	va_start(args, call);

#define   INT_ARG() va_arg(args, int)
#define  BOOL_ARG() (Celeste_P8_bool_t)va_arg(args, int)
#define RET_INT(_i)   do {ret = (_i); goto end;} while (0)
#define RET_BOOL(_b) RET_INT(!!(_b))

	//printf("%d\n",call);

	switch (call) {
	case CELESTE_P8_MUSIC: //music(idx,fade,mask)
		int index = INT_ARG();
		int fade = INT_ARG(); // I'll add fading another time
		mask = INT_ARG();

		(void)mask; //we do not care about this since the PSX keeps sounds and music separate (kinda)

		if (index == -1) { //stop playing
			CdControlB(CdlStop, 0, 0);
		}
		else if (mus[index / 10] >= 0)
		{
			track.track = loc[(index / 10)].track;
			track.minute = loc[(index / 10) ].minute;
			track.second = loc[(index / 10)].second;
			track.sector = 1;

			printf("Track %d  min %d sec %d sector %d   %d\n", track.track, track.minute, track.second, track.sector, index / 10);

			uint8_t s = 6;

			//CdControlB(CdlSetloc, &track, 0);
			CdControlB(CdlPlay, &s, 0);

			//DrawSync(0);
			VSync(0);

			currentMusic = mus[index / 10];
		}
		break;
	case CELESTE_P8_SPR:
		int sprite = INT_ARG();
		x = INT_ARG();
		y = INT_ARG();
		int cols = INT_ARG();
		int rows = INT_ARG();
		int flipx = BOOL_ARG();
		int flipy = BOOL_ARG(); // Not Supported / Used

		assert(rows == 1 && cols == 1);

		if (sprite >= 0) {

			int flip = flipx ? 64 : 0;

			sprt = (SPRT_8*)db_nextpri;

			setSprt8(sprt);
			setXY0(sprt, x + SCREEN_XOFFSET, y + SCREEN_YOFFSET);
			setUV0(sprt, 8 * (sprite % 16), 8 * (sprite / 16) + flip);
			setClut(sprt, 640, 240);
			setRGB0(sprt, 128, 128, 128);

			addPrim(&db[db_active].ot, sprt);
			sprt++;
			db_nextpri += sizeof(SPRT_8);

			if (currentTPage != 0) {
				tprit = (DR_TPAGE*)db_nextpri;

				setDrawTPage(tprit, 0, 1, getTPage(0 & 0x3, 0, 640, 0));
				addPrim(&db[db_active].ot, tprit);
				tprit++;

				db_nextpri += sizeof(DR_TPAGE);

				currentTPage = 0;
			}
		}
		break;
	case CELESTE_P8_BTN:
		b = INT_ARG();
		assert(b >= 0 && b <= 5);
		//FntPrint(0,"btn%04x\n", pad->btn);
		//FntFlush(-1);
		RET_BOOL(inputPoll(b));
		break;
	case CELESTE_P8_SFX:
		int id = INT_ARG();

		spu_playSFX(soundId2SPUChannel(id));

		break;
	case CELESTE_P8_PAL:
		int a = INT_ARG();
		b = INT_ARG();
		if (a >= 0 && a < 16 && b >= 0 && b < 16)
		{
			//swap palette colors
			palette[a].r = basePalette[b].r;
			palette[a].g = basePalette[b].g;
			palette[a].b = basePalette[b].b;
			palette[a].cd = basePalette[b].cd;
		}
		break;
	case CELESTE_P8_PAL_RESET:

		memcpy(palette, basePalette, sizeof(palette));

		break;
	case CELESTE_P8_CIRCFILL:
		int cx = INT_ARG() - camera_x;
		int cy = INT_ARG() - camera_y;
		int r = INT_ARG();
		col = INT_ARG();

		if (r <= 1)
		{
			RECT rect_a = { (cx - 1), cy, 3, 1 };
			RECT rect_b = { cx, (cy - 1), 1, 3 };

			p8_rectfill(rect_a.x, rect_a.y, rect_a.x + rect_a.h, rect_a.y + rect_a.w, col);
			p8_rectfill(rect_b.x, rect_b.y, rect_b.x + rect_b.h, rect_b.y + rect_b.w, col);
		}
		else if (r <= 2)
		{
			RECT rect_a = { (cx - 2), (cy - 1), 5, 3 };
			RECT rect_b = { (cx - 1), (cy - 2), 3, 5 };

			p8_rectfill(rect_a.x, rect_a.y, rect_a.x + rect_a.h, rect_a.y + rect_a.w, col);
			p8_rectfill(rect_b.x, rect_b.y, rect_b.x + rect_b.h, rect_b.y + rect_b.w, col);
		}
		else if (r <= 3)
		{
			RECT rect_a = { (cx - 3), (cy - 1), 7, 3 };
			RECT rect_b = { (cx - 1), (cy - 3), 3, 7 };
			RECT rect_c = { (cx - 2), (cy - 2), 5, 5 };

			p8_rectfill(rect_a.x, rect_a.y, rect_a.x + rect_a.h, rect_a.y + rect_a.w, col);
			p8_rectfill(rect_b.x, rect_b.y, rect_b.x + rect_b.h, rect_b.y + rect_b.w, col);
			p8_rectfill(rect_c.x, rect_c.y, rect_c.x + rect_c.h, rect_c.y + rect_c.w, col);
		}
		else  //i dont think the game uses this
		{
			/*int f = 1 - r; //used to track the progress of the drawn circle (since its semi-recursive)
			int ddFx = 1;   //step x
			int ddFy = -2 * r; //step y
			int x = 0;
			int y = r;

			//this algorithm doesn't account for the diameters
			//so we have to set them manually
			p8_line(cx, cy - y, cx, cy + r, col);
			p8_line(cx + r, cy, cx - r, cy, col);

			while (x < y)
			{
				if (f >= 0)
				{
					y--;
					ddFy += 2;
					f += ddFy;
				}
				x++;
				ddFx += 2;
				f += ddFx;

				//build our current arc
				p8_line(cx + x, cy + y, cx - x, cy + y, col);
				p8_line(cx + x, cy - y, cx - x, cy - y, col);
				p8_line(cx + y, cy + x, cx - y, cy + x, col);
				p8_line(cx + y, cy - x, cx - y, cy - x, col);
			}*/
		}
		break;
	case CELESTE_P8_PRINT:
		const char* str = va_arg(args, const char*);
		x = INT_ARG() - camera_x;
		y = INT_ARG() - camera_y;
		col = INT_ARG() % 16;

		p8_print(str, x, y, col);

		currentTPage = 1;
		break;
	case CELESTE_P8_RECTFILL:
		x0 = INT_ARG() - camera_x;
		y0 = INT_ARG() - camera_y;
		x1 = INT_ARG() - camera_x;
		y1 = INT_ARG() - camera_y;
		col = INT_ARG();

		p8_rectfill(x0, y0, x1, y1, col);
		break;
	case CELESTE_P8_LINE:
		x0 = INT_ARG() - camera_x;
		y0 = INT_ARG() - camera_y;
		x1 = INT_ARG() - camera_x;
		y1 = INT_ARG() - camera_y;
		col = INT_ARG();

		line = (LINE_F2*)db_nextpri;

		setLineF2(line);
		setXY2(line, x0 + SCREEN_XOFFSET, y0 + SCREEN_YOFFSET, x1 + SCREEN_XOFFSET, y1 + SCREEN_YOFFSET);
		setRGB0(line, palette[col].r, palette[col].g, palette[col].b);

		addPrim(&db[db_active].ot, line);
		line++;
		db_nextpri += sizeof(LINE_F2);

		break;
	case CELESTE_P8_MGET:
		tx = INT_ARG();
		int ty = INT_ARG();

		RET_INT(tilemap_data[tx + ty * 128]);
		break;
	case CELESTE_P8_FGET:
		int tile = INT_ARG();
		int flag = INT_ARG();

		RET_INT(gettileflag(tile, flag));
		break;
	case CELESTE_P8_CAMERA:
		camera_x = INT_ARG();
		camera_y = INT_ARG();
		break;
	case CELESTE_P8_MAP:
		int mx = INT_ARG(), my = INT_ARG();
		tx = INT_ARG(), ty = INT_ARG();
		int mw = INT_ARG(), mh = INT_ARG();
		mask = INT_ARG();

		for (x = 0; x < mw; x++)
		{
			for (y = 0; y < mh; y++)
			{
				int tile = tilemap_data[x + mx + (y + my) * 128];
				//hack
				if (mask == 0 || (mask == 4 && tile_flags[tile] == 4) || gettileflag(tile, mask != 4 ? mask - 1 : mask))
				{
					sprt = (SPRT_8*)db_nextpri;

					setSprt8(sprt);
					setXY0(sprt, (tx + x * 8 - camera_x) + SCREEN_XOFFSET, (ty + y * 8 - camera_y) + SCREEN_YOFFSET);
					setUV0(sprt, 8 * (tile % 16), 8 * (tile / 16));
					setClut(sprt, 640, 240);
					setRGB0(sprt, 128, 128, 128);

					addPrim(&db[db_active].ot, sprt);
					sprt++;
					db_nextpri += sizeof(SPRT_8);
				}
			}
		}
		tprit = (DR_TPAGE*)db_nextpri;

		setDrawTPage(tprit, 0, 1, getTPage(0 & 0x3, 0, 640, 0));
		addPrim(&db[db_active].ot, tprit);
		tprit++;

		db_nextpri += sizeof(DR_TPAGE);

		currentTPage = 0;
		break;
	}

end:
	va_end(args);
	return ret;
}