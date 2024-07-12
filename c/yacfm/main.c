#include "ncurses.h"
#include "unistd.h"
#include "limits.h"
#include "dirent.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "time.h"

#define MODE "NORMAL"


char* fileList[256];
int fileCount = 0;  
int position = 0;
int showHiddenFile = 0;


char* SelectedfileList[256];
int SelectedfileCount = 0;

char cwd[PATH_MAX];

void normalMode(char* current_path, char key);
void minimalMode();
void fancyMode();
void scanDir(char* arr[], int* count_var, char* dirname);
void printUsage();


int main(int argc, char* argv[]) {
  // Init
  initscr();
  raw();
  noecho();
  curs_set(0);
  start_color();
  use_default_colors();
  init_pair(1, COLOR_WHITE, COLOR_BLUE);
  char key = '-'; 

  //  Initialize path
  if(argc > 1) {
    if(!strcmp(argv[1], "--help")) {
      endwin();
      printUsage();
      return(0);
    }
    DIR* dir = opendir(argv[1]);
    if(dir != NULL) {
      chdir(argv[1]);
    }
    closedir(dir);
  } 
  getcwd(cwd, sizeof(cwd));

  // Read file from path
  scanDir(fileList, &fileCount, ".");

  // Set bg color
  wbkgd(stdscr, COLOR_PAIR(1));


  // Main Loop
  while(key != 'q') {
    refresh();
    
    if((strcmp(MODE, "NORMAL") == 0)) { normalMode(cwd, key); }
    //else if(strcmp(MODE, "MINIMAL")) { minimalMode(); }
    // else if(strcmp(MODE, "FANCY")) { fancyMode(); }


    key = getch();

  }

  clear();
  endwin();
  return 0;
}

// Implementation

void normalMode(char* current_path, char key) {

  if(key == 65) {
    if(position > 0) {
      position--;
    }
  } else if(key == 66) {
    if(position < fileCount - 1) {
      position++;
    }
  } else if(key == 10) {
    // Enter
    DIR* dir = opendir(fileList[position]);
    if(dir != NULL) {
      chdir(fileList[position]);
      scanDir(fileList, &fileCount, ".");
      getcwd(cwd, sizeof(cwd));
    } else {
      // Open file
    }
    closedir(dir);
    position = 0;
  } else if(key == 'h') {
    
    showHiddenFile = !showHiddenFile;
  }

  int currx = 1, curry = 1;

  WINDOW *topLeft = newwin(3, getmaxx(stdscr)/2 + 5, 1, 1);
  WINDOW *rightBox = newwin(getmaxy(stdscr) - 2, getmaxx(stdscr) - getmaxx(topLeft) - 4, 1, getmaxx(topLeft) + 2);
  WINDOW *leftBox = newwin(getmaxy(stdscr) - 10, getmaxx(stdscr)/2 + 5, 5, 1);
  WINDOW *infoBox = newwin(3, getmaxx(stdscr)/2 + 5, getmaxy(stdscr) - 4, 1);
 
  box(topLeft, 0, 0);
  box(rightBox, 0, 0);
  box(leftBox, 0, 0);
  box(infoBox, 0, 0);


  // topLeft
  mvwprintw(topLeft, 1, 1, current_path);

  scanDir(fileList, &fileCount, ".");

  // leftBox
  for(int x = 0; x < fileCount; x++) {
    if(x == position) {
      wattron(leftBox, A_REVERSE);
      mvwprintw(leftBox, curry, currx, fileList[x]); 
      wattroff(leftBox, A_REVERSE);
    } else { 
      mvwprintw(leftBox, curry, currx, fileList[x]); 
    }
    curry++;
  }

  // rightBox

  currx = 1;
  curry = 1;

  DIR* selectedDir = opendir(fileList[position]);
  if(selectedDir == NULL) {
    // File handling
    char fileBuffer[256];
    FILE* selectedFIle;
    selectedFIle = fopen(fileList[position], "r");
    while ((fgets(fileBuffer, 256, selectedFIle)) != NULL) {
      if(curry > getmaxy(rightBox) - 2) {
        break;
      }
      fileBuffer[getmaxx(rightBox) - 1] = '\0';
      mvwprintw(rightBox, curry++, currx, fileBuffer);
    }
    fclose(selectedFIle);
  } else {
    // Folder handling
    scanDir(SelectedfileList, &SelectedfileCount, fileList[position]);
    for(int x = 0; x < SelectedfileCount; x++) {
      mvwprintw(rightBox, curry, currx, SelectedfileList[x]);
      curry++;
    }
  }

  box(rightBox, 0, 0);

  wattron(rightBox, A_REVERSE);
  mvwprintw(rightBox, 0, getmaxx(rightBox)/2 - sizeof(" [PREVIEW] ")/2, "[ Preview ]");
  wattroff(rightBox, A_REVERSE);

  // InfoBox
  wattron(infoBox, A_REVERSE);
  mvwprintw(infoBox, 0, getmaxx(infoBox)/2 - sizeof(" [info] ")/2, "[ Info ]");
  wattroff(infoBox, A_REVERSE);

  // Print info
  
  char* filename = fileList[position]; 

  struct stat file_stat;
  if (stat(filename, &file_stat) == 0) {

    char access_time[20];
    char modify_time[20];
    char create_time[20];
        
    strftime(access_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_atime));
    strftime(modify_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));
    strftime(create_time, 20, "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_ctime));

    //mvwprintw(infoBox, 1, 1, "File Size: %.2f GB\n", file_size_gb);
    
    mvwprintw(infoBox, 1, 1, "Last Edit: %s\n", modify_time);
    if(file_stat.st_mtime == 0) {
      mvwprintw(infoBox, 1, 1, "Last Edit: No edit\n");
    }

    //mvwprintw(infoBox, 1, 2, "Created on: %s\n", create_time);
  }

  wrefresh(topLeft);
  wrefresh(rightBox);
  wrefresh(leftBox);
  wrefresh(infoBox);
}




void scanDir(char* arr[], int *count_var, char* dirname) {
  int i = 0;
  while(arr[i] != NULL) {
    arr[i] = NULL;
  }
  *count_var = 0;
  DIR* dir = opendir(dirname);
  struct dirent* entry;
  int x = 1;
  *count_var = *count_var + 1; //TODO if dir == / then dont add .. and add show unhide file option
  arr[0] = "..";
  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name, ".")  && strcmp(entry->d_name, "..")) {
      if((entry->d_name[0] == '.' && showHiddenFile == 1) || entry->d_name[0] != '.') {
        arr[x] = entry->d_name;
        *count_var = *count_var + 1;
        x++;
      }
    }
  }
  closedir(dir);
}

#include <stdio.h>

void printUsage() {
    fprintf(stdout,
        "Usage: yacfm [PATH]\n"
        "\n"
        "Description:\n"
        "yacfm is a file manager program that allows you to navigate and manage files and directories.\n"
        "\n"
        "Arguments:\n"
        "  [PATH]               Optional. Specify a path to open yacfm in a specific directory.\n"
        "                       If no path is provided, yacfm opens in the current directory.\n"
        "\n"
        "Options:\n"
        "  --help               Print this message and exit.\n"
        "\n"
        "Commands while running yacfm:\n"
        "  q                    Quit yacfm.\n"
        "  h                    Toggle show/hide hidden files.\n"
        "  Enter                Open selected file or directory.\n"
        "\n"
        "Examples:\n"
        "  yacfm                 Opens yacfm in the current directory.\n"
        "  yacfm /path/to/dir    Opens yacfm in the specified directory.\n"
    );
}
