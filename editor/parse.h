// parse.h -- text file parsing routines

#define	MAXTOKEN	1024

extern	char	token[MAXTOKEN];
extern	int		scriptline;

void	StartTokenParsing (char *data);
bool GetToken (bool crossline);
void UngetToken (void);
bool TokenAvailable (void);

