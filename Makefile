# MIT License, Copyright (c) 2020 Marvin Borner

all: compile clean

compile:
	@$(MAKE) clean --no-print-directory -C libc/
	@$(MAKE) libc --no-print-directory -C libc/
	@echo "Compiled libc"
	@$(MAKE) clean --no-print-directory -C libc/
	@$(MAKE) libk --no-print-directory -C libc/
	@echo "Compiled libk"
	@$(MAKE) --no-print-directory -C libgui/
	@echo "Compiled libgui"
	@$(MAKE) --no-print-directory -C kernel/
	@echo "Compiled kernel"
	@$(MAKE) --no-print-directory -C apps/
	@echo "Compiled apps"

test:
	@$(MAKE) clean --no-print-directory -C libc/
	@$(MAKE) libc --no-print-directory -C libc/
	@echo "Compiled libc"
	@$(MAKE) clean --no-print-directory -C libc/
	@$(MAKE) libk --no-print-directory -C libc/
	@echo "Compiled libk"
	@$(MAKE) --no-print-directory -C libgui/
	@echo "Compiled libgui"
	@$(MAKE) test --no-print-directory -C kernel/
	@echo "Compiled kernel"
	@$(MAKE) --no-print-directory -C apps/
	@echo "Compiled apps"

clean:
	@find kernel/ apps/ libc/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete
