# LAPORAN PENJELASAN SOAL SHIFT MODUL 4
## Soal 1

Semua nama file dan folder harus terenkripsi
Enkripsi yang Atta inginkan sangat sederhana, yaitu Caesar cipher. Namun, Kusuma mengatakan enkripsi tersebut sangat mudah dipecahkan. Dia menyarankan untuk character list diekspansi,tidak hanya alfabet, dan diacak. Berikut character list yang dipakai:

    qE1~ YMUR2"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\8s;g<{3.u*W-0

Misalkan ada file bernama “halo” di dalam folder “INI_FOLDER”, dan key yang dipakai adalah 17, maka:
“INI_FOLDER/halo” saat belum di-mount maka akan bernama “n,nsbZ]wio/QBE#”, saat telah di-mount maka akan otomatis terdekripsi kembali menjadi “INI_FOLDER/halo” .
Perhatian: Karakter ‘/’ adalah karakter ilegal dalam penamaan file atau folder dalam *NIX, maka dari itu dapat diabaikan

### Jawab :

#### Membuat Program Caesar Cipher
  Inisialisasikan character list dan shift.
```
    char char_list[400] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
    int i, n = strlen(fname), rot;
    if (mode == 1)            // variable mode menentukan apakah di-encrpyt atau di-decrypt
    {
      rot = KEY;              // jika encryt (mode = 1) shift yang dilakukan adalah key yang dipakai
    }else                     // jika decrypt (mode = -1) shift yang dilakukan adalah komplemen dari key
    {
      rot = 94 -KEY;
    }

```

  Lakukan shift terhadap kata yang diinputkan
```
    for(i = 0; i < n; i++)
    {
        if (fname[i]== '\0') break; 
	    	if (fname[i]== '/') continue;
        int j = 0;
        while ((char_list[j] == fname[i]) == 0) j++;  // cari character ke-i pada string yang diinputkan di character list
        j = (j + rot )% 94;       // lakukan shift berdasarkan key yang didapat sebleumnya
        fname[i] = char_list[j];

    }
```


### Mengenkripsi file di folder yang di-mount

Untuk mengenkripsi file, variable path yang merupakan argumen yang ada di fungsi operasi getattr perlu diencrypt.

```
        strcpy(fname, path);
        Caesar(fname,1);      // enkripsi path yang asli
        sprintf(fpath, "%s%s", dirpath, fname); // sambung dengan path dari root
```

Agar filesystem secara keseluruhan dapat berfungsi, selain di operasi getattr, path juga perlu dienkripsi di operasi fuse lain yang dipakai.

### Mendekripsi file saat di-mount
 
Untuk mendekripsi file saat di-mount, pada operasi readdir, lakukan dekripsi saat melakukan penelusuran folder dengan directory entry.

    dp = opendir(fpath);  // buka directory (nama directory sudah di enkripsi)
	  if (dp == NULL)
		  return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
    ....
    if(strcmp(de->d_name, ".")!=0 && strcmp(de->d_name, "..")!=0){  // lakukan dekripsi terhadap nama file yang sedang ditinjau
       Caesar(de->d_name,-1);

      res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}
 
### Output




## Soal 2 
Semua file video yang tersimpan secara terpecah-pecah (splitted) harus secara otomatis tergabung (joined) dan diletakkan dalam folder “Videos”
Urutan operasi dari kebutuhan ini adalah:
- Tepat saat sebelum file system di-mount
    - Secara otomatis folder “Videos” terbuat di root directory file system
    - Misal ada sekumpulan file pecahan video bernama “computer.mkv.000”, “computer.mkv.001”, “computer.mkv.002”, dst. Maka secara otomatis file pecahan tersebut akan di-join menjadi file video “computer.mkv”
    - Untuk mempermudah kalian, dipastikan hanya video file saja yang terpecah menjadi beberapa file.
    - Karena mungkin file video sangat banyak sehingga mungkin saja saat menggabungkan file video, file system akan membutuhkan waktu yang lama untuk sukses ter-mount. Maka pastikan saat akan menggabungkan file pecahan video, file system akan membuat 1 thread/proses(fork) baru yang dikhususkan untuk menggabungkan file video tersebut
    - Pindahkan seluruh file video yang sudah ter-join ke dalam folder “Videos”
    - Jangan tampilkan file pecahan di direktori manapun
- Tepat saat file system akan di-unmount
    - Hapus semua file video yang berada di folder “Videos”, tapi jangan hapus file pecahan yang terdapat di root directory file system
    - Hapus folder “Videos” 


### Jawab :


##### Membuat Fungsi Pengabung Video

   Menggunakan directory entry cari file awal dari video yang tersplit. Fungsi ini akan dipanggil setelah melakukan mkdir untuk membuat folder Videos di operasi init. Operasi init dipanggil sebelum mount.
    
    char vid[7] ="Videos";
    Caesar(vid,1);
    sprintf(path,"%s/%s",dirpath, vid);
    DIR *dir = opendir(dirpath);
    while ((dp = readdir(dir)) != NULL) // Lakukan readdir di root
        {
            char *cmp;
            struct stat buf;
          if ((cmp = strstr(Caesar(dp->d_name,-1), ".000")) != NULL) //cek jika nama file mengandung .000 (awal dari part file)
            {
                ....
                strcpy(curr,dp->d_name);
                curr[strlen(dp->d_name) - 4] = '\0'; // hapus extensi part file (.NNN)
                sprintf(existVid,"%s/%s",path, curr);
                if((stat (existVid, &buf) == 0)); // cek jika video sudah di join dan ditaruh di folder Videos
                else    // jika belum panggil thread untuk mengabung video
                {
                    flag = 1;
                    pthread_create(&tid[i], NULL, &JoinVid, (void *) curr); // kirim nama video(tanpa extensi part) ke thread
                    while(flag);
                    i++;
                }

            }
        } 
        while (i--) // join thread yang ada
        {
            pthread_join(tid[i],NULL);
        }
    
  Buatlah thread untuk mengabung video.
  
      flag = 0;
      char *tmp = (char*) args;
      char buf[1025*1025*2];
      int fd, fdDest; 
      char dest[400], destPath[400], matchName[400];
      strcpy(matchName,tmp);
      char vid[7] ="Videos";
      Caesar(vid,1);
      sprintf(dest,"%s/%s",dirpath, vid);
      sprintf(destPath,"%s/%s", dest, matchName);    
      struct dirent *dp;
      fdDest = open(destPath, O_CREAT |O_APPEND | O_RDWR, 0777); // buat file video yang inigin digabung di folder Videos
      DIR *dir = opendir(dirpath);
      int i = 0;
      while ((dp = readdir(dir)) != NULL)
      {
          char *cmp;
          if ((cmp = strstr(dp->d_name, matchName)) != NULL)  // cari part dari video tersebut
          {
              char curr[400], idenc[10];
              memset(curr,'\0',sizeof(curr));
              memset(idenc,'\0',sizeof(idenc));
              sprintf(idenc,".%03d",i);
              Caesar(idenc,1);
              sprintf(curr,"%s/%s%s",dirpath,matchName,idenc);
              fd = open(curr, O_RDWR); 
              write(fdDest, buf, read(fd, buf, sizeof(buf)));   // append part ke-i saat menemukan part dari video tersebut
              i++; 

          }
          close(fd);
      }      
      close(fdDest); 

#### Menyembunyikan file part di folder yang di-mount

  Untuk menyembunyikan file part, kita hanya perlu mengabaikan file dengan ekstensi video dan part saat proses directory entry di operasi readdir.
  
     while ((de = readdir(dp)) != NULL) {
        struct stat st;


        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
          ....
        if(strcmp(de->d_name, ".")!=0 && strcmp(de->d_name, "..")!=0){ // dekripsi nama file
           Caesar(de->d_name,-1);
               
        if (strstr(de->d_name, ".mp4.") || strstr(de->d_name, ".mov.") || strstr(de->d_name, ".mkv.")) 
            {         
               continue;   // Jika mengandung ekstensi video dan "." abaikan
            }
          res = (filler(buf, de->d_name, &st, 0));
          if(res!=0) break;
      }

#### Membuat Fungsi untuk menghapus folder Videos beserta isinya


  Dengan directory entry, hapus semua file yang berada di folder Videos, kemudian hapus folder Videos. Fungsi ini dipanggil di operasi destroy karena destroy dilakukan saat sebelum unmount.
  
    memset(path,'\0',sizeof(path));
    char vid[7] ="Videos";
    Caesar(vid,1);
    sprintf(path,"%s/%s",dirpath, vid);
    DIR *dir = opendir(path);   // buka folder Videos
    while ((dp = readdir(dir)) != NULL) // telusuri folder Videos
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            char curr[400];
            sprintf(curr,"%s/%s",path,dp->d_name);
            remove(curr); // hapus file yang ditinjau
        }
    }
    closedir(dir);
	rmdir(path);  // hapus folder Videos

### Output




## Soal 3
Sebelum diterapkannya file system ini, Atta pernah diserang oleh hacker LAPTOP_RUSAK yang menanamkan user bernama “chipset” dan “ic_controller” serta group “rusak” yang tidak bisa dihapus. Karena paranoid, Atta menerapkan aturan pada file system ini untuk menghapus “file bahaya” yang memiliki spesifikasi:
- Owner Name 	: ‘chipset’ atau ‘ic_controller’
- Group Name	: ‘rusak’
- Tidak dapat dibaca

Jika ditemukan file dengan spesifikasi tersebut ketika membuka direktori, Atta akan menyimpan nama file, group ID, owner ID, dan waktu terakhir diakses dalam file “filemiris.txt” (format waktu bebas, namun harus memiliki jam menit detik dan tanggal) lalu menghapus “file bahaya” tersebut untuk mencegah serangan lanjutan dari LAPTOP_RUSAK.


### Jawab :





## Soal 4 
Pada folder YOUTUBER, setiap membuat folder permission foldernya akan otomatis menjadi 750. Juga ketika membuat file permissionnya akan otomatis menjadi 640 dan ekstensi filenya akan bertambah “.iz1”. File berekstensi “.iz1” tidak bisa diubah permissionnya dan memunculkan error bertuliskan “File ekstensi iz1 tidak boleh diubah permissionnya.”



### Jawab :

### Output:

## soal 5
Ketika mengedit suatu file dan melakukan save, maka akan terbuat folder baru bernama Backup kemudian hasil dari save tersebut akan disimpan pada backup dengan nama namafile_[timestamp].ekstensi. Dan ketika file asli dihapus, maka akan dibuat folder bernama RecycleBin, kemudian file yang dihapus beserta semua backup dari file yang dihapus tersebut (jika ada) di zip dengan nama namafile_deleted_[timestamp].zip dan ditaruh kedalam folder RecycleBin (file asli dan backup terhapus). Dengan format [timestamp] adalah yyyy-MM-dd_HH:mm:ss

### Jawab :

### Output:

