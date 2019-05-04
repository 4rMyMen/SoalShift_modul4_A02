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
	gcc -Wall -pthread `pkg-config fuse --cflags` blabla.c -o output `pkg-config fuse --libs`
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
#include <pthread.h>
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
#include <pwd.h>
#include <grp.h>
#define KEY 31
#define DEC -1
#define ENC 1

static const char *dirpath = "/home/bimo/Desktop/loltest";


int flag;
pthread_t tid[4];
/////
// SOAL 1
char *Caesar(char fname[400], int mode)
{
    char char_list[400] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
    int i, n = strlen(fname), rot;
    if (mode == 1)
    {
      rot = KEY;
    }else
    {
      rot = 94 -KEY;
    }
    
    for(i = 0; i < n; i++)
    {
        if (fname[i]== '\0') break; 
		if (fname[i]== '/') continue;
        int j = 0;
        while ((char_list[j] == fname[i]) == 0) j++;
        j = (j + rot )% 94;
        fname[i] = char_list[j];

    }
    return fname;
}

void *JoinVid(void *args)
{
    flag = 0;
    char *tmp = (char*) args;
    char buf[1025*1025*2];
    int fd, fdDest; 
    char dest[400], destPath[400], matchName[400];
    memset(dest,'\0',sizeof(dest));
    memset(destPath,'\0',sizeof(destPath));
    memset(matchName,'\0',sizeof(matchName));
    strcpy(matchName,tmp);
    char vid[7] ="Videos";
    Caesar(vid,1);
    sprintf(dest,"%s/%s",dirpath, vid);
    sprintf(destPath,"%s/%s", dest, matchName);    
    struct dirent *dp;
    fdDest = open(destPath, O_CREAT |O_APPEND | O_RDWR, 0777); 
    DIR *dir = opendir(dirpath);
    int i = 0;
    while ((dp = readdir(dir)) != NULL)
    {
        char *cmp;
        if ((cmp = strstr(dp->d_name, matchName)) != NULL)
        {
            char curr[400], idenc[10];
            memset(curr,'\0',sizeof(curr));
            memset(idenc,'\0',sizeof(idenc));
			sprintf(idenc,".%03d",i);
			Caesar(idenc,1);
            sprintf(curr,"%s/%s%s",dirpath,matchName,idenc);
            fd = open(curr, O_RDWR); 
            write(fdDest, buf, read(fd, buf, sizeof(buf)));
            i++; 

        }
        close(fd);
    }      
    close(fdDest); 
    return (void*) 0;
}

void CheckVid()
{  
    
    char path[400];
    memset(path,'\0',sizeof(path));
    char vid[7] ="Videos";
    Caesar(vid,1);
    sprintf(path,"%s/%s",dirpath, vid);


    struct dirent *dp;
    DIR *dir = opendir(dirpath);
    int i = 0;
    while ((dp = readdir(dir)) != NULL)
    {
        char *cmp;
        struct stat buf;
        if ((cmp = strstr(Caesar(dp->d_name,-1), ".000")) != NULL)
        {
            Caesar(dp->d_name,1);
            char curr[400], existVid[400];
            memset(curr,'\0',sizeof(curr));
            memset(existVid,'\0',sizeof(existVid));
            strcpy(curr,dp->d_name);
            curr[strlen(dp->d_name) - 4] = '\0';
            sprintf(existVid,"%s/%s",path, curr);

            if((stat (existVid, &buf) == 0));
            else
            {
                // JoinVid(curr);
                flag = 1;
                pthread_create(&tid[i], NULL, &JoinVid, (void *) curr);
                while(flag);
                i++;
                
            }

        }
    } 
    while (i--)
    {
        pthread_join(tid[i],NULL);
    }
    
    closedir(dir);    


}

void DelAll()
{
    flag = 0;
    struct dirent *dp;
    char path[400];
    memset(path,'\0',sizeof(path));
       char vid[7] ="Videos";
    Caesar(vid,1);
    sprintf(path,"%s/%s",dirpath, vid);
    DIR *dir = opendir(path);
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            char curr[400];
            sprintf(curr,"%s/%s",path,dp->d_name);
            remove(curr);
        }
    }
    closedir(dir);
	rmdir(path);
}
// SOAL 2
/////


static void *xmp_init(struct fuse_conn_info *conn)
{
    (void) conn;
    
   char vid[7] ="Videos";
    char path[400];
    memset(path,'\0',sizeof(path));
    Caesar(vid,1);
    sprintf(path,"%s/%s",dirpath, vid);
    mkdir(path,0777);
    CheckVid();
    return NULL;
}
void xmp_destroy(void *private_data)
{
  DelAll();

}




static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[400], fname[400];

    if(strcmp(path, ".")!=0 && strcmp(path, "..")!=0)
	{
        memset(fname, '\0', sizeof(fname));
        strcpy(fname, path);
        Caesar(fname,1);
        sprintf(fpath, "%s%s", dirpath, fname);
    }
    else
	{
	    sprintf(fpath,"%s%s",dirpath,path);
    }
	
    res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[400], fname[400];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else{
        memset(fname, '\0', sizeof(fname));
        strcpy(fname, path);
        Caesar(fname,1);
        sprintf(fpath, "%s%s", dirpath, fname);
    }
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
        char fstat[400];

		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
        memset(fstat, '\0', sizeof(fstat));
        sprintf(fstat, "%s/%s", fpath, de->d_name);

        stat(fstat, &st);

        struct passwd *pw= getpwuid(st.st_uid);
        struct group  *gr= getgrgid(st.st_gid);
        struct tm *taimu= localtime(&st.st_atime);

        if(strcmp(de->d_name, ".")!=0 && strcmp(de->d_name, "..")!=0){
            Caesar(de->d_name,-1);
            if(((strcmp(pw->pw_name, "chipset")==0||strcmp(pw->pw_name, "ic_controller")==0)&&strcmp(gr->gr_name, "rusak")==0))
            {
                FILE *miris;
                char file[400];
                char out[400];
                sprintf(file, "%s/V[EOr[c[Y`HDH", fpath);
                sprintf(out, "\nNama:%s, Owner:%s, Group:%s, Atime:%04d-%02d-%02d_%02d:%02d:%02d\n", de->d_name, pw->pw_name, gr->gr_name, taimu->tm_year+1900, taimu->tm_mon+1, taimu->tm_mday, taimu->tm_hour, taimu->tm_min, taimu->tm_sec);

                miris=fopen(file, "a+");
                fputs(out, miris);
                fclose(miris);
                remove(fstat);
            }
        }
        if (strstr(de->d_name, ".mp4.") || strstr(de->d_name, ".mov.") || strstr(de->d_name, ".mkv."))
        {
           continue;
        }
        
        res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
     Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	int res = 0;
 	int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{   
	int fd;
	int res;
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);


	return res;
}

static int xmp_rmdir(const char *path)
{
	int res;
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}


static int xmp_rename(const char *from, const char *to)
{
	int res;
    char enfrom[400], ento[400];
    char frpath[400], topath[400];

    memset(enfrom, '\0', sizeof(enfrom));
    memset(ento, '\0', sizeof(ento));
    strcpy(enfrom, from);
    strcpy(ento, to);
    Caesar(enfrom,1);
    Caesar(ento,1);
    sprintf(frpath, "%s%s", dirpath, enfrom);
    sprintf(topath, "%s%s", dirpath, ento);

	res = rename(frpath, topath);
	if (res == -1)
		return -errno;

	return 0;
}


static int xmp_unlink(const char *path)
{
	int res;
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}


static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}
static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
    char fpath[400], fname[400];
    
    memset(fname, '\0', sizeof(fname));
	strcpy(fname, path);
	Caesar(fname,1);
	sprintf(fpath, "%s%s", dirpath, fname);
	res = mkdir(fpath, mode);

    if(strstr("@ZA>AXio",fpath)==0)
    {
            res=mkdir(fpath,0750);
    }
    else
    {
            res=mkdir(fpath,mode);
    }

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
    char fpath[400], fname[400];//, npath[400];


    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);

	Caesar(fname,1);
	sprintf(fpath, "%s%s", dirpath, fname);
    //sprintf(npath, "%s/@ZA>AXio/", fpath);
	res = chmod(fpath, mode);

    if(strstr(fpath,"@ZA>AXio")==0)
    {
		if(strstr(fpath,"`[S%")){
			pid_t child;
			child=fork();
			if(child==0){
				char *argv[]={"zenity","--warning","--text='File ekstensi iz1 tidak boleh diubah permissionnya.'",NULL};
				execv("/usr/bin/zenity",argv);
			}
			return 0;
		}
		else{
			res=chmod(fpath,mode);
		}
    }
    else{
        res=chmod(fpath,mode);
    }
	if (res == -1)
		return -errno;   

    return 0;
}


static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
    char fpath[400], fname[400];

    memset(fname, '\0', sizeof(fname));
    strcpy(fname, path);
    Caesar(fname,1);
    sprintf(fpath, "%s%s", dirpath, fname);

	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}


static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    char iz [400];
    //char npath [400];
    (void) fi;

    int res;
    char fpath[400], fname[400];
    memset(fname, '\0', sizeof(fname));
	strcpy(fname, path);
	Caesar(fname,1);
	sprintf(fpath, "%s%s", dirpath, fname);
    //sprintf(npath, "%s@ZA>AXio/", fpath);
	res = creat(fpath, mode);

    if(strstr("/@ZA>AXio/",fpath)==0)
    {   
		res=creat(fpath,0640);
		pid_t child;
		child=fork();
		if(child==0){
			strcpy(iz,fpath);
			strcat(iz,"`[S%");
			char *argv[]={"mv",fpath,iz,NULL};
			execvp(argv[0],argv);
		}
		return 0;
    }
    else
    {
		res=creat(fpath,mode);
    }

    if(res == -1)
	    return -errno;

    close(res);

    return 0;
}

static struct fuse_operations xmp_oper = {
	.init     	= xmp_init,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.mkdir		= xmp_mkdir,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.create   	= xmp_create,
	.destroy  = xmp_destroy,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
