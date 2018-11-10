struct timer {
    unsigned int div;   // divider
    unsigned int tima;  // timer counter
    unsigned int tma;   // timer module
    unsigned char tac;  // timer controller
    unsigned int speed;
    unsigned int started; 
    unsigned int tick;
};

extern struct timer timer;

void setDiv(unsigned char value);
void setTima(unsigned char value);
void setTma(unsigned char value);
void setTac(unsigned char value);

unsigned int getDiv(void);
unsigned int getTima(void); 
unsigned int getTma(void);
unsigned int getTac(void);

void tick(void);
void timerCycle(void);
