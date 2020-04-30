#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/fs/ata.h>
#include <kernel/fs/ext2.h>
#include <kernel/fs/vfs.h>
#include <kernel/system.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/stdlib.h>

static struct ext2_superblock superblock;
static struct bgd *bgdt;
static size_t block_size;
static size_t num_groups;
static void read_block(uint32_t block_num, void *buf);
static void load_superblock();
static void load_bgdt();
static void read_inode(struct ext2_inode *inode, uint32_t inode_num);

void ext2_init_fs()
{
	load_superblock();
	load_bgdt();

	struct ext2_inode root_inode;
	read_inode(&root_inode, ROOT_INODE);
	debug("Creation time: %d", root_inode.creation_time);
	debug("UID: %d", root_inode.uid);
	debug("Type & perms: 0x%x", root_inode.type_and_permissions);
	debug("Size: %d", root_inode.size);

	debug("Files:");

	struct ext2_file file;
	ext2_open_inode(ROOT_INODE, &file);
	struct ext2_dirent dirent;

	while (ext2_next_dirent(&file, &dirent))
		debug("Inode %d, name '%s'", dirent.inode_num, dirent.name);

	kfree(file.buf);
}

static void read_block(uint32_t block_num, void *buf)
{
	uint32_t lba = block_num * block_size / SECTOR_SIZE;
	size_t sectors = block_size / SECTOR_SIZE;

	read_abs_sectors(lba, sectors, buf);
}

static void load_superblock()
{
	uint16_t buf[SUPERBLOCK_LENGTH / 2];

	read_abs_sectors(SUPERBLOCK_LBA, SUPERBLOCK_SECTORS, buf);
	memcpy(&superblock, buf, sizeof(struct ext2_superblock));

	block_size = 1024 << superblock.log2_block_size;
	num_groups = superblock.total_blocks / superblock.blocks_per_group;

	assert(superblock.signature == EXT2_SIGNATURE);
	debug("Total inodes: 0x%x", superblock.total_inodes);
	debug("Total blocks: 0x%x", superblock.total_blocks);
	debug("Drive size: %dMiB", (block_size * superblock.total_blocks) >> 20);
	debug("Block size: %d", block_size);
	debug("Num blocks: %d", superblock.total_blocks);
	debug("Blocks/group: %d", superblock.blocks_per_group);
	debug("Inodes/group: %d", superblock.inodes_per_group);
	debug("Num groups: %d", num_groups);
	debug("Creator OS: %s", superblock.creator_os_id == 0 ? "Linux" : "Other");
}

static void load_bgdt()
{
	size_t bgdt_sectors = (sizeof(struct bgd) * num_groups) / SECTOR_SIZE + 1;
	size_t bgdt_block = (SUPERBLOCK_OFFSET + SUPERBLOCK_LENGTH) / block_size + 1;
	uint32_t bgdt_lba = bgdt_block * block_size / SECTOR_SIZE;

	uint16_t buf[bgdt_sectors * SECTOR_SIZE / 2];
	read_abs_sectors(bgdt_lba, bgdt_sectors, buf);

	size_t bgdt_size = sizeof(struct bgd) * num_groups;
	bgdt = kmalloc(bgdt_size);
	memcpy(bgdt, buf, bgdt_size);
}

static void read_inode(struct ext2_inode *inode, uint32_t inode_num)
{
	inode_num--;
	size_t block_group = inode_num / superblock.inodes_per_group;

	struct bgd *bgd = &bgdt[block_group];
	uint32_t i_table_block = bgd->inode_table_addr;

	size_t index = inode_num % superblock.inodes_per_group;
	size_t block_offset = (index * INODE_SIZE) / block_size;
	size_t offset_in_block = (index * INODE_SIZE) % block_size;
	size_t block = i_table_block + block_offset;

	size_t num_sectors = sizeof(struct ext2_inode) / SECTOR_SIZE + 1;
	uint16_t buf[num_sectors * SECTOR_SIZE / 2];
	read_abs_sectors(block * block_size / SECTOR_SIZE, num_sectors, buf);
	memcpy(inode, &buf[offset_in_block / 2], sizeof(struct ext2_inode));
}

void ext2_open_inode(uint32_t inode_num, struct ext2_file *file)
{
	read_inode(&file->inode, inode_num);
	file->pos = 0;
	file->block_index = 0;
	file->buf = kmalloc(block_size);
	file->curr_block_pos = 0;

	read_block(file->inode.dbp[0], file->buf);
}

size_t ext2_read(struct ext2_file *file, uint8_t *buf, size_t count)
{
	if (file->pos + count > file->inode.size)
		count = file->inode.size - file->pos;

	size_t bytes_left = count;

	while (bytes_left > 0) {
		size_t to_copy = bytes_left;

		bool new_block = file->curr_block_pos + to_copy >= block_size;
		if (new_block)
			to_copy = block_size - file->curr_block_pos;

		memcpy(buf + (count - bytes_left), file->buf + file->curr_block_pos, to_copy);
		file->curr_block_pos += to_copy;
		file->pos += to_copy;
		bytes_left -= to_copy;

		if (new_block) {
			file->curr_block_pos = 0;
			file->block_index++;
			if (file->block_index >= 12) {
				// TODO: Add triple block pointer support
				uint32_t *tmp = kmalloc(block_size);
				read_block(file->inode.ibp, tmp);
				read_block(tmp[file->block_index - 12], file->buf);
			} else {
				read_block(file->inode.dbp[file->block_index], file->buf);
			}
		}
	}

	return count;
}

#define READ_SIZE (sizeof(struct ext2_dirent) - sizeof(uint8_t *))

bool ext2_next_dirent(struct ext2_file *file, struct ext2_dirent *dir)
{
	uint8_t buf[READ_SIZE];
	if (ext2_read(file, buf, READ_SIZE) != READ_SIZE)
		return false;

	memcpy(dir, buf, READ_SIZE);

	size_t size = dir->name_len + 1;
	uint8_t *name = kmalloc(size);
	if (ext2_read(file, name, size - 1) != size - 1)
		return false;

	dir->name = name;
	dir->name[size - 1] = '\0';

	size_t bytes_left = dir->total_len - (READ_SIZE + size - 1);
	if (bytes_left > 0) {
		uint8_t dummy[bytes_left];
		ext2_read(file, dummy, bytes_left);
	}

	return true;
}

uint32_t ext2_find_in_dir(uint32_t dir_inode, const char *name)
{
	uint32_t inode;
	struct ext2_file dir;
	struct ext2_dirent dirent;

	ext2_open_inode(dir_inode, &dir);
	while (ext2_next_dirent(&dir, &dirent)) {
		if (strcmp((char *)dirent.name, name) == 0) {
			inode = dirent.inode_num;
			goto cleanup;
		}
	}

	inode = 0;

cleanup:
	kfree(dir.buf);
	return inode;
}

uint32_t ext2_look_up_path(char *path)
{
	if (path[0] != '/')
		return 0;

	path++;
	uint32_t curr_dir_inode = ROOT_INODE;

	while (1) {
		size_t j;
		for (j = 0; path[j] != '/' && path[j] != '\0'; j++)
			;

		if (path[j] == '\0')
			break;

		path[j] = '\0';
		curr_dir_inode = ext2_find_in_dir(curr_dir_inode, path);
		path[j] = '/';

		if (curr_dir_inode == 0)
			return 0;

		path += j + 1;
	}

	uint32_t inode = ext2_find_in_dir(curr_dir_inode, path);
	if (inode == 0)
		return 0;

	return inode;
}

// Interface

uint8_t *read_file(char *path)
{
	uint32_t inode = ext2_look_up_path(path);
	struct ext2_file file;
	ext2_open_inode(inode, &file);
	if (inode != 0) {
		size_t size = file.inode.size;
		debug("Reading %s: %dKiB", path, size >> 10);
		uint8_t *buf = kmalloc(size);
		ext2_read(&file, buf, size);
		kfree(file.buf);
		buf[size - 1] = '\0';
		return buf;
	} else {
		warn("File not found");
		return NULL;
	}
}

void ext2_vfs_open(struct fs_node *node)
{
	node->inode = ext2_look_up_path(node->name);

	if (node->inode != 0) {
		struct ext2_file *file = kmalloc(sizeof *file);
		ext2_open_inode(node->inode, file);

		node->impl = file;

		// TODO: More file metadata
	}
}

void ext2_vfs_close(struct fs_node *node)
{
	kfree(node->impl);
}

uint32_t ext2_vfs_read(struct fs_node *node, size_t offset, size_t size, char *buf)
{
	if (offset != ((struct ext2_file *)node->impl)->pos) {
		panic("Seeking is currently unsupported for Ext2 files\n");
		return 0;
	}

	return (uint32_t)ext2_read(node->impl, (uint8_t *)buf, size);
}

uint32_t ext2_vfs_write(struct fs_node *node, size_t offset, size_t size, char *buf)
{
	panic("Writing to Ext2 is currently unsupported\n");

	return 0;
}

struct dirent *ext2_vfs_read_dir(struct fs_node *node, size_t index)
{
	struct ext2_dirent ext2_dir;

	if (ext2_next_dirent(node->impl, &ext2_dir)) {
		struct dirent *dir = kmalloc(sizeof *dir);

		dir->inode = ext2_dir.inode_num;
		strcpy(dir->name, (char *)ext2_dir.name);

		return dir;
	} else {
		return NULL;
	}
}

struct fs_node *ext2_vfs_find_dir(struct fs_node *node, char *name)
{
	uint32_t inode = ext2_find_in_dir(node->inode, name);
	if (inode == 0) {
		return NULL;
	} else {
		struct fs_node *found = kmalloc(sizeof *found);
		found->inode = inode;

		return found;
	}
}

void ext2_mount(struct fs_node *mountpoint)
{
	assert(mountpoint->node_ptr == NULL && (mountpoint->type & MOUNTPOINT_NODE) == 0);
	assert((mountpoint->type & DIR_NODE) != 0);

	struct fs_node *ext2_root = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	ext2_root->name[0] = '\0';
	ext2_root->permissions = 0;
	ext2_root->uid = 0;
	ext2_root->gid = 0;
	ext2_root->inode = ROOT_INODE;
	ext2_root->length = 0;
	ext2_root->type = DIR_NODE;
	ext2_root->read = ext2_vfs_read;
	ext2_root->write = ext2_vfs_write;
	ext2_root->open = ext2_vfs_open;
	ext2_root->close = ext2_vfs_close;
	ext2_root->read_dir = ext2_vfs_read_dir;
	ext2_root->find_dir = ext2_vfs_find_dir;
	ext2_root->node_ptr = NULL;
	ext2_root->impl = NULL;

	mountpoint->type |= MOUNTPOINT_NODE;
	mountpoint->node_ptr = ext2_root;
}