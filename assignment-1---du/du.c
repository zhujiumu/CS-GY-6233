#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int isInteger(char* srcstr) {
    for (int i = 0; i < strlen(srcstr); i++) {
        if (srcstr[i] < '0' || srcstr[i] > '9') {
            return 0;
        }
    }
    return 1;
}

char *fmtname(char *path) {
  static char buf[DIRSIZ + 1];
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

void du(char *path, int k_flag, int t_flag, int t_threshold, uint* combinedSize, uint* combinedBlockSize, int r_flag, int* argv_check) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  
  if ((fd = open(path, 0)) < 0) {
    printf(2, "check usage.\n");
    return;
  }

  if (fstat(fd, &st) < 0) {
    printf(2, "check usage.\n");
    close(fd);
    return;
  }

  switch (st.type) {
  case T_FILE:
    printf(1, "%d %s\n", st.size, path);
    printf(1, "%d %s\n", st.size, path);
    *combinedSize = *combinedSize + st.size;
    *combinedBlockSize = *combinedBlockSize + (st.size + 512 - 1) / 512;
    *argv_check = 1;
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf(1, "check usage.\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (stat(buf, &st) < 0) {
        printf(1, "check usage.\n");
        continue;
      }
      
      if (fmtname(buf)[0] == '.') {
          continue;
      } else if (st.type == 3){
          continue;
      } else if (st.type == 1){
          if (!r_flag) {
              continue;
          } else {
              du(buf, k_flag, t_flag, t_threshold, combinedSize, combinedBlockSize, r_flag, argv_check);
          };
      }else {
          int byte_size = st.size;
          int block_size = (st.size + 512 - 1) / 512;
          //char* name_to_display = fmtname(buf);

          if (t_flag == 1) {
              if (k_flag == 1) {
                  if (byte_size > t_threshold) {
                    printf(1, "%d %s\n", block_size, buf);
                    *combinedSize = *combinedSize + st.size;
                    *combinedBlockSize = *combinedBlockSize + block_size;
                  }
              } else {
                  if (byte_size > t_threshold) {
                    printf(1, "%d %s\n", byte_size, buf);
                    *combinedSize = *combinedSize + st.size;
                    *combinedBlockSize = *combinedBlockSize + block_size;
                  }
              }
          } else {
              if (k_flag == 1) {
                  printf(1, "%d %s\n", block_size, buf);
              } else {
                  printf(1, "%d %s\n", byte_size, buf);
              }
              *combinedSize = *combinedSize + st.size;
              *combinedBlockSize = *combinedBlockSize + block_size;
          }

      }
      
    }
    

    break;
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int k_flag = 0;
  int t_flag = 0;
  int t_threshold = 0;
  int r_flag = 0;
  int argv_check = 0;
  int cntr = 1;
  
  uint combinedSize = 0;
  uint combinedBlockSize = 0;
  
  while (1) {
    if (argv[cntr][0] == '-') {
      switch(argv[cntr][1]) {
          case 'k':
            //printf(1, "block instead of bytes \n");
            if (k_flag == 1) {
                printf(1, "check usage.\n");
                exit();
            }
            k_flag = 1;
            break;

          case 't':
            //printf(1, "max threshold applied \n");
            if (t_flag == 1) {
                printf(1, "check usage.\n");
                exit();
            }

            t_flag = 1;
            cntr++;
            if (isInteger(argv[cntr]) == 0) {
                printf(1, "check usage.\n");
                exit();
            }
            t_threshold = atoi(argv[cntr]);
            break;

          case 'r':
            //printf(1, "block instead of bytes \n");
            if (r_flag == 1) {
                printf(1, "check usage.\n");
                exit();
            }

            r_flag = 1;
            break;

          default:
            printf(1, "check usage.\n");
            exit();
      }

    } else {
        break;
    }
    cntr++;

  }
  
  //printf(1, "%d %d %d %d %s\n", cntr, k_flag, t_flag, t_threshold, argv[cntr]);

  //checking if du is following options..filename, only 1 dir/filename should be passed
  if (!(cntr == argc - 1 || cntr == argc)) {
    printf(1, "check usage.\n");
    exit();

  }
  
  //checking if additional parameters are passed at the end
  if (argv[cntr][0] == '-') {
      printf(1, "check usage.\n");
      exit();
  }

  //run du function  
  char* path_name = "";
  if (cntr == argc) {
      path_name = ".";

  } else {
      strcpy(path_name, argv[cntr]);
  }

  if (path_name[strlen(path_name) - 1] == '/') {
      path_name[strlen(path_name) - 1]= '\0';
  }

  du(path_name, k_flag, t_flag, t_threshold, &combinedSize, &combinedBlockSize, r_flag, &argv_check);

  if (argv_check == 1) {
      exit();
  }
  
  char* path_orig = ".";
  if (argc != cntr) {
      path_orig = argv[cntr];
  }

  if (k_flag == 1) {
      printf(1, "%d %s\n", combinedBlockSize, path_orig) ;
  } else {
      printf(1, "%d %s\n", combinedSize, path_orig);
  }

  exit();

}
