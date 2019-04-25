/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Minor modifications and note by Andy Sayler (2012) <www.andysayler.com>
  Source: fuse-2.8.7.tar.gz examples directory
  http://sourceforge.net/projects/fuse/files/fuse-2.X/
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
  gcc -Wall `pkg-config fuse --cflags` fusexmp.c -o fusexmp `pkg-config fuse --libs`
  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.
*/

#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h> 
#include <string.h>
#include <sys/types.h>
#define KEY 17
#define DEC -1
#define ENC 1
char char_list[] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";

static const char *dirpath = "/home/paksi/Downloads";
char old[1000];
char update[1000];
char nama1[100];
char nama2[100];
FILE *fdir1;
FILE *fdir2;
FILE *inside;

/////
// SOAL 1
char *Caesar(char *fname, int mode)
{

    int i, n = strlen(fname);
    for(i = 0; i < n; i++)
    {
        int j = 0;
        while ((char_list[j] == fname[i]) == 0) j++;
        j = (j + (KEY * mode) )% 94;
        fname[i] = char_list[j];

    }
    puts(fname);
    return fname;
}

void RecursiveRename(char *basePath, int mode)
{
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            
            struct stat sb;
            char curr[1000];
            sprintf(curr,"%s/%s",basePath,dp->d_name);
            char new[1000];
            char *csr = Caesar(dp->d_name, mode);
            sprintf(new,"%s/%s",basePath,csr);
            if (stat(curr, &sb) == 0 && S_ISDIR(sb.st_mode))
            {
                sprintf(path,"%s",curr);
                RecursiveRename(path,mode);
                rename( path , new );

            }
            else
            {
                sprintf(path,"%s",curr);
                rename( path, new );
            }
            
 
        }
    }

    closedir(dir);
}

static void *xmp_init(struct fuse_conn_info *conn)
{
	(void) conn;
  char path[1000]; 
  sprintf(path,"%s", dirpath);
  RecursiveRename(path, DEC);
	return NULL;
}
void xmp_destroy(void *private_data)
{
  char path[1000];
  sprintf(path,"%s", dirpath);
  RecursiveRename(path, ENC);

}

// SOAL 1
/////

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];

	if(strcmp(path,"/") == 0)
    {
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
	else sprintf(fpath, "%s%s",dirpath,path);

	fdir1 = fopen (fpath, "r");
	fgets(old, 1000, fdir1);

	int res = 0;
 	int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	fclose(fdir1);
	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
    char path2[1000];
	char *extention;
	char taimu[100];
    time_t rawtime;
    struct tm *info;

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

    int res;
	int fd;

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;


	fdir2 = fopen (fpath, "r");

	fgets(update, 1000, fdir2);

	if(strcmp(old,update)!=0){
		DIR *dir;
        char newdir[100]="/home/paksi/Downloads/Backup";

        dir = opendir(newdir);
        if (dir==NULL)
        {
            char *create[]={"mkdir", "/home/paksi/Downloads/Backup", NULL};
            execv("/bin/mkdir", create);
            closedir(dir);
        }

        time ( &rawtime );
		info = localtime ( &rawtime );
		sprintf(taimu, "%04d-%02d-%02d_%02d:%02d:%02d", info->tm_year+1900, info->tm_mon+1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec);
		extention=strchr(path,'.');
		sprintf(path2, "%s", path);
		int i=strlen(path2)-4;
		if(path2[i]=='.')path2[i]='\0';

	    sprintf(nama1,"/home/paksi/Downloads%s",path);
        sprintf(nama2, "/home/paksi/Downloads/Backup%s_%s%s", path2, taimu, extention);

		inside=fopen(nama2,"w+");
		fprintf(inside, "%s", update);
		fclose(inside);
	}

	fclose(fdir2);
	close(fd);

	return res;
}

static int xmp_truncate(const char *path, off_t size)
{

	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);

	int res;

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}


static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
	.destroy	= xmp_destroy,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.write 		= xmp_write,
	.truncate  	= xmp_truncate
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
