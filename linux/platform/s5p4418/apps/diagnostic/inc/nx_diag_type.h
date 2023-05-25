#ifndef __NX_DIAG_TYPE_H__
#define	__NX_DIAG_TYPE_H__

#define	NUM_APP_IN_SCREEN	6

#define	SCREEN_WIDTH		1024
#define	SCREEN_HEIGHT		600

enum {
	TEST_FAILED = -1,
	TEST_NOTTESTED = 0,
	TEST_PASSED = 1,
};

enum{
	BTN_ACT_NONE,
	BTN_ACT_OK,
	BTN_ACT_NOK,
	BTN_ACT_NEXT,
	BTN_ACT_PREV,
	BTN_ACT_EXEC,

	BTN_ACT_PLAY,
	BTN_ACT_STOP,
	BTN_ACT_PAUSE,
	BTN_ACT_REC,

	BTN_ACT_EXIT,

	BTN_ACT_DOWN,
	BTN_ACT_UP,
	BTN_ACT_TOGGLE,

	ACT_VIBRATION,
	ACT_UPDATE,
};

typedef struct tagNXDiagPluginInfo NXDiagPluginInfo;
typedef struct tagNXDiagAppType NXDiagAppType;
typedef struct tagNXDiagButton NXDiagButton;

typedef int (*TestFunc)(int);

struct tagNXDiagAppType{
    char *name    ;		//	test item name
	char *so_name ;		//	shared object file name
	char *symbol  ;		//	symbol name
	char *log_file;		//	log file name
    TestFunc api  ;		//	symbol function
	int	result    ;		//	test result
	tagNXDiagAppType *next;
};

typedef enum {
	BUTTON_DISABLED ,
	BUTTON_NORMAL   ,
	BUTTON_PUSHED   ,
	BUTTON_FOCUS_OUT,
	BUTTON_FOCUS_IN ,
} BUTTON_STATE;

struct tagNXDiagButton{
	BUTTON_STATE		state;
	int					action;
	SDL_Rect			rect;
	void				*priv;		//	private information
	NXDiagPluginInfo	*plugin;
	tagNXDiagButton		*next;
};

//	Plugin Information
typedef NXDiagPluginInfo*	(*GetPluginInfo)(void);
//	Test DiagPlugin
typedef int (*TestDiagPlugin)( int );
typedef int (*AutoTestPlugin)( void );

struct tagNXDiagPluginInfo{
	char				name[20];
	char				description[128];
	char				fileName[256];
	TestDiagPlugin		testDiag;
	AutoTestPlugin		AutoTest;
	int					result;
};


#define	PLUGIN_FUNC_SYMBOL	"NXGetPluginInfo"


#endif //__NX_DIAG_TYPE_H__
