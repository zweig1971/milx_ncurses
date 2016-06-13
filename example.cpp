#include <stdlib.h>
#include <string.h>
//#include <panel.h>
#include <unistd.h>
#include "scumil.h"
#include <curses.h>

#define ENTER 10
#define ESCAPE 27

bool scu_select;
bool ifk_select;

char scu_adress[80];
char ifk_nummber[80];

scu_mil myscu;

WINDOW *menubar,*messagebar, *statusbar;


void init_curses()
{
        initscr();
        start_color();
        init_pair(1,COLOR_WHITE,COLOR_BLUE);
        init_pair(2,COLOR_BLUE,COLOR_WHITE);
        init_pair(3,COLOR_RED,COLOR_WHITE);
	init_pair(4,COLOR_WHITE,COLOR_YELLOW);
	init_pair(5,COLOR_RED,COLOR_YELLOW);
        curs_set(0);
        noecho();
        keypad(stdscr,TRUE);
}

void draw_menubar(WINDOW *menubar)
{
        wbkgd(menubar,COLOR_PAIR(2));
        waddstr(menubar,"SCU");
        wattron(menubar,COLOR_PAIR(3));
        waddstr(menubar,"(F2)");
        wattroff(menubar,COLOR_PAIR(3));
        wmove(menubar,0,20);
        waddstr(menubar,"IFK");
        wattron(menubar,COLOR_PAIR(3));
        waddstr(menubar,"(F3)");
       wattroff(menubar,COLOR_PAIR(3));
        wmove(menubar,0,40);
        waddstr(menubar,"Extra");
        wattron(menubar,COLOR_PAIR(3));
        waddstr(menubar,"(F4)");
	wattroff(menubar,COLOR_PAIR(3));
}


WINDOW **draw_menu_scu(int start_col)
{
        WINDOW **items;
        items=(WINDOW **)malloc(2*sizeof(WINDOW *));

        items[0]=newwin(10,19,1,start_col);
        wbkgd(items[0],COLOR_PAIR(2));
        box(items[0],ACS_VLINE,ACS_HLINE);

        items[1]=subwin(items[0],1,17,2,start_col+1);

        wprintw(items[1],"SCU to select",1);

        wbkgd(items[1],COLOR_PAIR(1));
        wrefresh(items[0]);
        return items;
}


WINDOW **draw_menu_ifk(int start_col)
{
        WINDOW **items;
        items=(WINDOW **)malloc(4*sizeof(WINDOW *));

        items[0]=newwin(10,19,1,start_col);
        wbkgd(items[0],COLOR_PAIR(2));
        box(items[0],ACS_VLINE,ACS_HLINE);

        items[1]=subwin(items[0],1,17,2,start_col+1);
        items[2]=subwin(items[0],1,17,3,start_col+1);
        items[3]=subwin(items[0],1,17,4,start_col+1);

	wprintw(items[1],"IFK Online",1);
	wprintw(items[2],"IFK Diagnostics",2);
	wprintw(items[3],"IFK Commands",3);

        wbkgd(items[1],COLOR_PAIR(1));
        wrefresh(items[0]);
        return items;
}


WINDOW **draw_menu_extra(int start_col)
{
        WINDOW **items;
        items=(WINDOW **)malloc(2*sizeof(WINDOW *));

        items[0]=newwin(10,19,1,start_col);
        wbkgd(items[0],COLOR_PAIR(2));
        box(items[0],ACS_VLINE,ACS_HLINE);

        items[1]=subwin(items[0],1,17,2,start_col+1);

        wprintw(items[1],"EXIT",1);

        wbkgd(items[1],COLOR_PAIR(1));
        wrefresh(items[0]);
        return items;
}


void delete_menu(WINDOW **items,int count)
{
        int i;
        for (i=0;i<count;i++)
                delwin(items[i]);
        free(items);
}


int scroll_menu(WINDOW **items,int count,int menu_start_col)
{
        int key;
        int selected=0;
        while (1) {
                key=getch();
                if (key==KEY_DOWN || key==KEY_UP) {
                        wbkgd(items[selected+1],COLOR_PAIR(2));
                        wnoutrefresh(items[selected+1]);
                        if (key==KEY_DOWN) {
                                selected=(selected+1) % count;
                        } else {
                                selected=(selected+count-1) % count;
                        }
                        wbkgd(items[selected+1],COLOR_PAIR(1));
                        wnoutrefresh(items[selected+1]);
                        doupdate();
                } else if (key==ESCAPE) {
                        return -1;
                } else if (key==ENTER) {
                        return selected;
                }
        }
}


void get_userinput(char *userinput)
{
	int row,col;
   
	getmaxyx(stdscr,row,col);
	echo();

	move(row-3,0);
	waddstr(stdscr,"> ");
	curs_set(1);
	getstr(userinput);
	move(row-3,0);
	clrtobot();
	noecho();
	curs_set(0);
}


void exit(WINDOW *menubar,WINDOW *messagebar, WINDOW *statusbar)
{
	delwin(menubar);
	delwin(messagebar);
	delwin(statusbar);
	endwin();
}


void draw_messagebar(WINDOW *base)
{
	wbkgd(base,COLOR_PAIR(2));
}


void draw_statusbar(WINDOW *base)
{
        wbkgd(base,COLOR_PAIR(4));
}


void get_scuadress(WINDOW *messagebar)
{
	DWORD status = 0;
	DWORD error_status = 0;
	string error_str;

	char error_char[80];
  
	wprintw(messagebar,"Insert SCU adress. Example :tcp/scuxl0089.acc");
	touchwin(stdscr);
	refresh();


	sprintf(scu_adress, "%.20s", "tcp/scuxl0089.acc");
//	get_userinput(scu_adress);

	werase(messagebar);
	wprintw(messagebar,"Try connecting :%s",scu_adress); 
	touchwin(stdscr);
        refresh();

        status = myscu.scu_milbusopen(scu_adress, error_status);
        error_str = myscu.scu_milerror(status);

	sprintf(error_char, "%.20s", error_str.c_str());

	werase(messagebar);
        wprintw(messagebar,"open scu :%s",error_char);	
	touchwin(stdscr);
        refresh();

	if (status != status_ok){
		scu_select = false;
		ifk_select = false;
	}
	else {
                scu_select = true;
                ifk_select = false;
	}

}


WINDOW **draw_ifk_dia()
{
        WINDOW **ifkwin;

	//getmaxyx(stdscr,row,col);

	ifkwin=(WINDOW **)malloc(2*sizeof(WINDOW *));

	ifkwin[0]=newwin(30,50,2,2);

        //ifkwin[0]=newwin(30,50,int((row/2)-20),int((col/2)-35));
        wbkgd(ifkwin[0],COLOR_PAIR(2));
        box(ifkwin[0],ACS_VLINE,ACS_HLINE);

        wmove(ifkwin[0],1,1);
        waddstr(ifkwin[0],"IFKs cnt: 0");
	wmove(ifkwin[0],3,1);
	waddstr(ifkwin[0],"IFKs adr.online      IFKs adr.unstable");
        wrefresh(ifkwin[0]);
	return ifkwin;
}


WINDOW **draw_ifk_command(int start_col)
{
        WINDOW **items;
        items=(WINDOW **)malloc(8*sizeof(WINDOW *));

        items[0]=newwin(10,19,2,start_col);
        wbkgd(items[0],COLOR_PAIR(2));
        box(items[0],ACS_VLINE,ACS_HLINE);

        items[1]=subwin(items[0],1,17,3,start_col+1);
        items[2]=subwin(items[0],1,17,4,start_col+1);
        items[3]=subwin(items[0],1,17,5,start_col+1);
	items[4]=subwin(items[0],1,17,6,start_col+1);
        items[5]=subwin(items[0],1,17,7,start_col+1);
        items[6]=subwin(items[0],1,17,8,start_col+1);

        wprintw(items[1],"Write data",1);
        wprintw(items[2],"Write cmd",2);
        wprintw(items[3],"Read Data",3);
        wprintw(items[4],"Write IFK",4);
        wprintw(items[5],"Read IFK",5);
        wprintw(items[6],"Echo test",6);

        wbkgd(items[1],COLOR_PAIR(1));
        wrefresh(items[0]);
        return items;
}


WINDOW **draw_ifk_menue()
{
	WINDOW **ifkwin;
        
        //getmaxyx(stdscr,row,col);

        ifkwin=(WINDOW **)malloc(2*sizeof(WINDOW *));


	ifkwin[0]=newwin(30,30,2,2);
        //ifkwin[0]=newwin(30,30,int((row/2)-20),int((col/2)-35));
        wbkgd(ifkwin[0],COLOR_PAIR(2));
        box(ifkwin[0],ACS_VLINE,ACS_HLINE);

        wmove(ifkwin[0],1,1);
        waddstr(ifkwin[0],"IFKs cnt: 0");
        wmove(ifkwin[0],3,1);
        waddstr(ifkwin[0],"IFKs found (hex):");

        wrefresh(ifkwin[0]);
        return ifkwin;

}


void statusbar_update(WINDOW * statusbar)
{
	string scu_str(scu_adress);
	string ifk_str(ifk_nummber);
	string output_str;
	char output_char[80];

	if(!scu_select){
		scu_str= "SCU : NOT CONNECTED";
	}
	else{
		scu_str= "SCU : "+scu_str;
	}


	if(!ifk_select){
		ifk_str= "IFK : NOT SELECTED";
	}
	else{
		ifk_str= "IFK : 0x"+ifk_str;
	}
	
	output_str = scu_str+ "     "+ ifk_str;
	sprintf(output_char, "%.70s", output_str.c_str());

	wbkgd(statusbar,COLOR_PAIR(5));
        werase(statusbar);
        wprintw(statusbar,"%s",output_char);

	touchwin(stdscr);
        refresh();

	usleep(500000);

        wbkgd(statusbar,COLOR_PAIR(4));
        werase(statusbar);
        wprintw(statusbar,"%s",output_char);

        touchwin(stdscr);
        refresh();
}


int scan_milbus(int ifk_online[])
{
        int countmax    = 256;
        int index       = 1;
	int found       = 0;
        WORD returnifkad= 0;

        DWORD errorstatus = status_ok;

        do
        {
                if(myscu.scu_milbus_ifk_on (index, returnifkad, errorstatus))
                {
			found++;
                        ifk_online[found] = returnifkad;
			
                }
                index++;

        }while (index < countmax);
	
	return(found);
}


void ifk_online()
{
        WINDOW **ifkwin;

	int ifk_online[256];
	int ifk_found;
	int index = 0;

	string userinput_str;

        if (!scu_select){
                statusbar_update(statusbar);
                return;
        }

        ifkwin=draw_ifk_menue();
	ifk_found = scan_milbus(ifk_online);
	
	if (ifk_found == 0){

        	wmove(ifkwin[0],4,1);
        	waddstr(ifkwin[0],"NOTHING THERE");
        	wrefresh(ifkwin[0]);
	}
	else {

		wmove(ifkwin[0],1,11);
		wprintw(ifkwin[0], "%d", ifk_found);

		do{
			index++;
			wmove(ifkwin[0],4+index,1);
			wprintw(ifkwin[0], "%03x", ifk_online[index]);
	
		}while (index < ifk_found);

		wrefresh(ifkwin[0]);
	}
	
	wprintw(messagebar,"Enter IFK adress or press enter to exit.");
	wrefresh(messagebar);

	get_userinput(ifk_nummber);
	userinput_str = string(ifk_nummber);
	
	if (userinput_str.size() != 0){
		ifk_select= true;
	} 
	else {
		ifk_select= false;
	}

	werase(messagebar);
	statusbar_update(statusbar);
        touchwin(stdscr);
        refresh();
}


void ifk_dia()
{
        WINDOW **ifkwin;

        int ifk_online_a[256];
	int ifk_online_b[256];
	int ifk_unstable[256];
        int ifk_found_a = 0;
	int ifk_found_u = 0;
	int ifk_found_b = 0;
	int index   = 0;
        int index_a = 0;
	int index_b = 0;
	int index_u = 0;
	bool ifk_found = false;
	int key;


        if (!scu_select){
                statusbar_update(statusbar);
                return;
        }

        ifkwin=draw_ifk_dia();
	werase(messagebar);
        wprintw(messagebar,"Press BACKSPACE to clear. Press ESC to exit.");
        wrefresh(messagebar);

	index = 0;
	timeout(1);

	// liste unstable init
	do{
		index++;
		ifk_unstable[index] = 0;
	}while(index < 255);

	do{
		index = 0;

		// listen init
		do{
			index++;
			ifk_online_a[index] = 0;
			ifk_online_b[index] = 0;

		}while(index < 255);

		// ifks am bus ermitteln	
		ifk_found_a = scan_milbus(ifk_online_a);

		// counter anzeigen
		wmove(ifkwin[0],1,11);
		wprintw(ifkwin[0],"   ");
		wmove(ifkwin[0],1,11);
                wprintw(ifkwin[0], "%d", ifk_found_a);		
	
                index_a = 0;

		// ausgabe loeschen
                do{
                        index_a++;
                        wmove(ifkwin[0],4+index_a,2);
                        wprintw(ifkwin[0],"   ");

			wmove(ifkwin[0],4+index_a,23);
                        wprintw(ifkwin[0],"   ");

                        wrefresh(ifkwin[0]);

                }while (index_a < 20);

		index_a = 0;		
		
		// ifks anzeigen
                do{
                        index_a++;
                        wmove(ifkwin[0],4+index_a,2);
                        wprintw(ifkwin[0], "%03x", ifk_online_a[index_a]);
			wrefresh(ifkwin[0]);

                }while (index_a < ifk_found_a);

		// ifks am bus ermitteln 
		ifk_found_b = scan_milbus(ifk_online_b);
		index_a = 0;
		
		// liste A mit B vergleichen
		do{
			index_a++;
			index_b = 0;
			ifk_found = false;

			do{
				index_b++;
				if(ifk_online_a[index_a] == ifk_online_b[index_b]){
					index_b = 255;
					ifk_found = true;			
				}

			}while(index_b < 255);	

			// IFK wurde nicht in Liste gefunden -> in liste unstable aufnehmen
			if(!ifk_found){
								
				index_u = 0;
				ifk_found = false;

				// pruefen ob wert schon in liste
				do{
					index_u++;
					if(ifk_unstable[index_u] == ifk_online_a[index_a]){
                                        	index_b = ifk_found_u;
                                        	ifk_found = true;
					}

				}while(index_u < ifk_found_u);
				
				// wenn wert nicht gefunden -> in liste aufnehmen
				if(!ifk_found){
					ifk_found_u++;
					ifk_unstable[ifk_found_u] = ifk_online_a[index_a];	
				}
			}

			index_u = 0;

			// wenn eine unstable ifk gefunden -> ausgabe
			if(ifk_found_u != 0){
				do{
					index_u++;
					wmove(ifkwin[0],4+index_u,23);
					wprintw(ifkwin[0], "%03x", ifk_unstable[index_u]);
	                        	wrefresh(ifkwin[0]);

				}while (index_u < ifk_found_u);
			};

		}while(index_a < 255);

		key=getch();
		if(key== ESCAPE) {
			timeout(-1);
			return;
		} else if(key== KEY_BACKSPACE){

			index = 0;
			ifk_found_u = 0;
			do{
	                	index++;
                		ifk_unstable[index] = 0;
       			 }while(index < 255);
		}

	}while(true);
}


void write_data()
{
	char data_mil[5];
	int  data_int = 0;

	string data_test_str;

	DWORD status = status_ok;
	DWORD errorstatus = status_ok;

	werase(messagebar);
        wprintw(messagebar,"Write data to Mil Bus. Enter data in hex or press Enter to exit");
        wrefresh(messagebar);
 
	get_userinput(data_mil);

	// wurde nur enter gedrueckt -> ende
	data_test_str = string(data_mil);

        if (data_test_str.size() == 0){

                werase(messagebar);
                wprintw(messagebar,"Choose your destiny. Press ESC to exit.");
                wrefresh(messagebar);
                return;
        }


	// convert char hex to intger
	data_int = (int)strtol(data_mil, NULL, 16);

	// auf milbus schreiben
	status =  myscu.scu_milbus_write_data(WORD(data_int), errorstatus);

	// ausgabe ergebniss
	werase(messagebar);
        wprintw(messagebar,"Sending data 0x%04x Status %04x:", data_int, status);
        wrefresh(messagebar);
}


void write_cmd()
{
        char cmd_mil[5];
        int  cmd_int   = 0;
	int  ifkadr_int = 0;

        string cmd_test_str;

        DWORD status = status_ok;
        DWORD errorstatus = status_ok;

        werase(messagebar);
        wprintw(messagebar,"Write CMD to Mil Bus. Enter CMD in hex or press Enter to exit");
        wrefresh(messagebar);

	get_userinput(cmd_mil);

        // wurde nur enter gedrueckt -> ende
        cmd_test_str = string(cmd_mil);

        if (cmd_test_str.size() == 0){

                werase(messagebar);
                wprintw(messagebar,"Choose your destiny. Press ESC to exit.");
                wrefresh(messagebar);
                return;
        }

        // convert char hex to intger
        cmd_int	  = (int)strtol(cmd_mil, NULL, 16);
	ifkadr_int = (int)strtol(ifk_nummber, NULL, 16);

	// auf milbus schreiben
	status = myscu.scu_milbus_write_cmd(BYTE(cmd_int), BYTE(ifkadr_int),errorstatus);

        // ausgabe ergebniss
        werase(messagebar);
        wprintw(messagebar,"Sending CMD 0x%02x to IFK 0x%02x Status %04x:", cmd_int, ifkadr_int, status);
        wrefresh(messagebar);
}


void read_data()
{
        DWORD status = status_ok;
        DWORD errorstatus = status_ok;
	WORD data = 0;

	// milbus lesen
	status=myscu.scu_milbus_read_data(data, errorstatus);

        // ausgabe ergebniss
        werase(messagebar);
        wprintw(messagebar,"Reading data from MilBus 0x%04x  Status %04x:", data, status);
        wrefresh(messagebar);
}


void write_ifk()
{
        DWORD errorstatus = status_ok;
        DWORD status = status_ok;

	char data_char[5];
	char cmd_char[5];

        int data_int   = 0;
        int cmd_int    = 0;
	int ifkadr_int = 0;

	string temp_str;

        werase(messagebar);
        wprintw(messagebar,"Write data to IFK. Enter data in hex or press Enter to exit");
        wrefresh(messagebar);

	// data einlesen
	get_userinput(data_char);

        // wurde nur enter gedrueckt -> ende
        temp_str = string(data_char);

        if (temp_str.size() == 0){

                werase(messagebar);
                wprintw(messagebar,"Choose your destiny. Press ESC to exit.");
                wrefresh(messagebar);
                return;
        }


        werase(messagebar);
        wprintw(messagebar,"Enter CMD in hex or press Enter to exit");
        wrefresh(messagebar);

	//cmd einlesen
	get_userinput(cmd_char);

        // wurde nur enter gedrueckt -> ende
        temp_str = string(cmd_char);

        if (temp_str.size() == 0){

                werase(messagebar);
                wprintw(messagebar,"Choose your destiny. Press ESC to exit.");
                wrefresh(messagebar);
                return;
        }

        // convert char hex to intger
        cmd_int    = (int)strtol(cmd_char, NULL, 16);
        data_int   = (int)strtol(data_char, NULL, 16);
	ifkadr_int = (int)strtol(ifk_nummber, NULL, 16);

        // auf milbus schreiben
	status = myscu.scu_milbus_ifk_wr (BYTE(ifkadr_int), BYTE (cmd_int), WORD (data_int), errorstatus);
	
        // ausgabe ergebniss
        werase(messagebar);
        wprintw(messagebar,"Sending data 0x%04x CMD 0x%02x to IFK 0x%02x Status %04x:", data_int, cmd_int, ifkadr_int ,status);
        wrefresh(messagebar);
}


void read_ifk()
{
        DWORD errorstatus = status_ok;
        DWORD status = status_ok;
        WORD data_int = 0;

        int cmd_int  = 0;
	int ifkadr_int = 0;
	
        char cmd_char[5];
	string temp_str;
	
        werase(messagebar);
        wprintw(messagebar,"Read data from IFK. Enter CMD in hex or press Enter to exit");
        wrefresh(messagebar);

        //cmd einlesen
        get_userinput(cmd_char);

        // wurde nur enter gedrueckt -> ende
        temp_str = string(cmd_char);

        if (temp_str.size() == 0){

                werase(messagebar);
                wprintw(messagebar,"Choose your destiny. Press ESC to exit.");
                wrefresh(messagebar);
                return;
        }

        // convert char hex to intger
        cmd_int    = (int)strtol(cmd_char, NULL, 16);
        ifkadr_int = (int)strtol(ifk_nummber, NULL, 16);

	// vom milbus lesen
        status = myscu.scu_milbus_ifk_rd (BYTE(ifkadr_int), BYTE (cmd_int), data_int, errorstatus);

        // ausgabe ergebniss
        werase(messagebar);
        wprintw(messagebar,"Reading data 0x%04x with CMD 0x%02x from IFK 0x%02x Status %04x:", data_int, cmd_int, ifkadr_int ,status);
        wrefresh(messagebar);
}


void ifk_command()
{
	WINDOW **menu_items;
	int selected_item;

       	if (!scu_select){
                statusbar_update(statusbar);
                return;
        }

        werase(messagebar);
        wprintw(messagebar,"Choose your destiny. Press ESC to exit.");
        wrefresh(messagebar);

	do{

        	menu_items=draw_ifk_command(2);
        	selected_item=scroll_menu(menu_items,6,5);
        	delete_menu(menu_items,6);

		if (selected_item == 0){
			write_data();
		}

                if (selected_item == 1){
		        if (!ifk_select){
				statusbar_update(statusbar);
                	} else write_cmd();
                }
 
                if (selected_item == 2){
                        read_data();
                }


                if (selected_item == 3){
                        if (!ifk_select){
                                statusbar_update(statusbar);
                        } else write_ifk();
                }


                if (selected_item == 4){
                        if (!ifk_select){
                                statusbar_update(statusbar);
                        } else read_ifk();
                }



        }while(selected_item >= 0);

	werase(messagebar);
	wrefresh(messagebar);
}



int main()
{
	int key;
	int row,col;

	scu_select= false;
	ifk_select= false;

	init_curses();

	bkgd(COLOR_PAIR(1));

	getmaxyx(stdscr,row,col);

	menubar=subwin(stdscr,1,col,0,0);
	messagebar=subwin(stdscr,1,col,row-4,0);   
	statusbar=subwin(stdscr,1,col,row-5,0);

	draw_menubar(menubar);
	draw_statusbar(statusbar);
	draw_messagebar(messagebar);

	statusbar_update(statusbar);	
        
	touchwin(stdscr);
        refresh();

/*

//	draw_ifk_menue(ifkwin);
//	refresh();
//	getchar();

//	test();
	touchwin(stdscr);
	refresh();
	getchar();
*/

	move(row-6,0);
	printw("Press F2,F3 or F4 to open the menus. ");
	printw("ESC quits.");

	refresh();

	do {
		int selected_item;
		WINDOW **menu_items;

		key=getch();
        	werase(messagebar);
        	wrefresh(messagebar);

        	if (key==KEY_F(2)) {
            		menu_items=draw_menu_scu(0);
            		selected_item=scroll_menu(menu_items,1,0);
            		delete_menu(menu_items,2);
            		if (selected_item<0)
                		wprintw(messagebar,"You haven't selected any item.");
            		else{
				get_scuadress(messagebar);
				statusbar_update(statusbar);
            		}
            		touchwin(stdscr);
            		refresh();
        	} else if (key==KEY_F(3)) {
            		menu_items=draw_menu_ifk(20);
            		selected_item=scroll_menu(menu_items,3,5);
            		delete_menu(menu_items,3);
            		if (selected_item<0)
                		wprintw(messagebar,"You haven't selected any item.");
            		else{
                		wprintw(messagebar,
                  		"You have selected menu item %d.",selected_item+1);
				if (selected_item == 0)
				{
					werase(messagebar);
					wrefresh(messagebar);
					touchwin(stdscr);
					refresh();
					ifk_online();
				}
				if (selected_item == 1){
					werase(messagebar);
                                        wrefresh(messagebar);
                                        touchwin(stdscr);
                                        refresh();
					ifk_dia();
				}
              			 if (selected_item == 2){
                                        werase(messagebar);
                                        wrefresh(messagebar);
                                        touchwin(stdscr);
                                        refresh();
                                        ifk_command();
                                }
	
			}
            		touchwin(stdscr);
            		refresh();
       		 } else if (key==KEY_F(4)) {
            		menu_items=draw_menu_extra(40);
            		selected_item=scroll_menu(menu_items,5,6);
            		delete_menu(menu_items,2);
            	if (selected_item<0)
                	wprintw(messagebar,"You haven't selected any item.");
            	else {
			wprintw(messagebar,"Have a nice day ! ");
                	touchwin(stdscr);
                	refresh();
			usleep(500000);
                	exit(menubar, messagebar, statusbar);
			return 0;
	    		}
            		touchwin(stdscr);
            		refresh();
        	}

    	} while (true);
   
  
 
    	exit(menubar, messagebar, statusbar);
    	return 0;
}
