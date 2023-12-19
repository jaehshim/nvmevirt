#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <time.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <linux/nvme_ioctl.h>

#define PAGE_SIZE 4096
#define NUM 1

int main(int argc, char ** argv) {
	int fd;
	int i;
	int num;
	char * ptr;
	int ret;
	int buffer_size = PAGE_SIZE * NUM;
	__u32 write_offset = 8192;
	__u32 read_offset = 16384;

	fd = open(argv[1], O_RDWR | O_DIRECT | O_SYNC);
	if (fd == -1)
		printf("Error\n");

	char * read_data = (char *)memalign(PAGE_SIZE, buffer_size);
	char * write_data = (char *)memalign(PAGE_SIZE, buffer_size);
	char * buffer = (char *)memalign(PAGE_SIZE, buffer_size);

	memset(read_data, 0, buffer_size);
	memset(write_data, 0, buffer_size);
	memset(buffer, 0, buffer_size);

	for (i = 0; i < buffer_size; i++) {
		read_data[i] = 'R';
	}
	for (i = 0; i < buffer_size; i++) {
		write_data[i] = 'W';
	}

	/* Initalize page to read */
	ret = pwrite(fd, read_data, buffer_size, read_offset);
	printf("Initalize: data filled with %d 'R's to offset(%d)\n", ret, read_offset);


	/* Initalize RW command */
	struct nvme_passthru_cmd nvmeCmd;
	struct nvme_passthru_cmd *m_nvmeCmd = &nvmeCmd;
	memset(m_nvmeCmd, 0, sizeof(struct nvme_passthru_cmd));

	m_nvmeCmd->nsid = 1;
	m_nvmeCmd->opcode = 0x66;
	m_nvmeCmd->addr = (__u64) write_data;
	m_nvmeCmd->data_len = buffer_size;
	m_nvmeCmd->cdw10 = write_offset / 512;
	m_nvmeCmd->cdw12 = buffer_size / 512 - 1;
	m_nvmeCmd->cdw13 = buffer_size / 512 - 1;
	m_nvmeCmd->cdw14 = read_offset / 512;

	ret = ioctl(fd, NVME_IOCTL_IO_CMD, m_nvmeCmd);
	if (ret < 0) {
		printf("ioctl failed!! %d\n", ret);
	}

	/* Verify RW command READ */
	num = 0;
	for (int i = 0; i < buffer_size; i++) {
		if (write_data[i] == 'R')
			num++;
	}
	printf("RW command retrieved data filled with %d 'R's from offset(%d)\n", num, read_offset);

	/* Verify RW command WRITE*/
	ret = pread(fd, buffer, buffer_size, write_offset);
	num = 0;
	for (int i = 0; i < buffer_size; i++) {
		if (buffer[i] == 'W')
			num++;
	}
	printf("RW command wrote data filled with %d 'W's from offset(%d)\n", num, write_offset);

	free(read_data);
	free(write_data);
	free(buffer);
	close(fd);

	return 0;
}


