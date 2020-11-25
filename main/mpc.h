typedef struct {
	int16_t volume;         // 47
	int16_t repeat;         // 0
	int16_t random;         // 0
	int16_t single;         // 0
	int16_t consume;        // 0
	int16_t playlist;       // 8
	int16_t playlistlength; // 10
	float mixrampdb;        // 0.000000
	char state[10];         // play stop pause
	int16_t song;           // 4
	int16_t songidi;        // 780
	char time[10];          // 10:243
	float elapsed;          // 9.543
	int16_t bitrate;        // 192
	float duration;         // 243.278
	float audio;            // 44100:24:2
	int16_t nextsong;       // 5
	int16_t nextsongid;     // 781
} STATUS_t;

typedef struct {
	char file[128];         // Frankie Rose/Frankie Rose - 2012 - Interstellar/05-frankie_rose-pair_of_wings.mp3
	char Last_Modified[30]; // 2020-11-05T08:41:47Z
	char Artist[64];        // Frankie Rose
	char AlbumArtist[64];   // Frankie Rose
	char Title[64];         // Pair Of Wings
	char Album[64];         // Interstellar
	int16_t Track;          // 5
	char Date[20];          // 2012-00-00
	char Genre[64];         // Indie
	char Composer[64];      // Wu Li Leung
	int16_t Time;           // 243
	float duration;         // 243.278
	int16_t Pos;            // 4
	int16_t Id;             // 780
} CURRENTSONG_t;
