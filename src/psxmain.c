#include <psxapi.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <psxspu.h>
#include <psxcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "celeste.h"

static unsigned char ramAddr[1000000];

#define PICO8_W 128
#define PICO8_H 128

// OT and Packet Buffer sizes
#define OT_LEN			4094
#define PACKET_LEN		20768

// Screen resolution
#define SCREEN_XRES		320
#define SCREEN_YRES		240

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







static CVECTOR basePalette[16]{
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

static void* initial_game_state = NULL;

static uint16_t buttonState = 0;



void mainLoop();

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

u_long sendVAGtoRAM(unsigned int VAG_data_size, unsigned char* VAG_data) {
    u_long size;
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);                              // DMA transfer; can do other processing during transfer
    size = SpuWrite((uint32_t*)VAG_data + sizeof(VAGhdr), VAG_data_size);     // transfer VAG_data_size bytes from VAG_data  address to sound buffer
    SpuIsTransferCompleted(SPU_TRANSFER_WAIT);                     // Checks whether transfer is completed and waits for completion
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

	TIM_IMAGE* tim = { 0 };
	uint32_t* tex = LoadFile(filename);

	GetTimInfo(tex, tim);

	// Upload pixel data to framebuffer
	LoadImage(tim->prect, tim->paddr);
	DrawSync(0);

	// Upload CLUT to framebuffer
	if (tim->mode & 0x8) {
		LoadImage(tim->crect, tim->caddr);
		DrawSync(0);
	}

	free(tex);
}

void LoadSound(const char* filename, unsigned long channel) {
	uint32_t* data = LoadFile(filename);

	spu_LoadVAG((u_long*)data, channel);

	free(data);
}

void LoadData() {

	LoadTexture("\\GFX.TIM;1");
	LoadTexture("\\FONT.TIM;1");

	LoadSound("\\snd0.VAG;1", SPU_01CH);
	LoadSound("\\snd1.VAG;1", SPU_02CH);
	LoadSound("\\snd2.VAG;1", SPU_03CH);
	LoadSound("\\snd3.VAG;1", SPU_04CH);
	LoadSound("\\snd4.VAG;1", SPU_05CH);
	LoadSound("\\snd5.VAG;1", SPU_06CH);
	LoadSound("\\snd6.VAG;1", SPU_07CH);
	LoadSound("\\snd7.VAG;1", SPU_08CH);
	LoadSound("\\snd8.VAG;1", SPU_09CH);
	LoadSound("\\snd9.VAG;1", SPU_10CH);
	LoadSound("\\snd13.VAG;1", SPU_11CH);
	LoadSound("\\snd14.VAG;1", SPU_12CH);
	LoadSound("\\snd15.VAG;1", SPU_13CH);
	LoadSound("\\snd16.VAG;1", SPU_14CH);
	LoadSound("\\snd23.VAG;1", SPU_15CH);
	LoadSound("\\snd35.VAG;1", SPU_16CH);
	LoadSound("\\snd37.VAG;1", SPU_17CH);
	LoadSound("\\snd38.VAG;1", SPU_18CH);
	LoadSound("\\snd40.VAG;1", SPU_19CH);
	LoadSound("\\snd50.VAG;1", SPU_20CH);
	LoadSound("\\snd51.VAG;1", SPU_21CH);
	LoadSound("\\snd54.VAG;1", SPU_22CH);
	LoadSound("\\snd55.VAG;1", SPU_23CH);

	printf("Assets Loaded\n");
}

int main(void) {
	int pico8emu(CELESTE_P8_CALLBACK_TYPE call, ...);

	init();

	LoadData();

	Celeste_P8_set_call_func(pico8emu);

	initial_game_state = malloc(Celeste_P8_get_state_size());
	if (initial_game_state) {
		Celeste_P8_save_state(initial_game_state);
	}

	Celeste_P8_set_rndseed(rand());

	Celeste_P8_init();

	while (1) {
		mainLoop();
	}
}

void mainLoop() {

}