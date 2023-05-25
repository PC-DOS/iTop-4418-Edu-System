########################################################################
# Set Install Directory
ifndef	INSTALL_DIR
INSTALL_DIR := ./__install
endif

########################################################################
# Module Definition & Version Management
DEVICE_NAME := S5P4418
MOD_BASE := diagnostic
MOD_NAME := s5p4418_diag

BUILD_NUMBER_FILE = build_number_$(MOD_NAME)
BUILD_INFO_FILE = inc/build_info_$(MOD_NAME).h
MAJOR = 0
MINOR = 1


######################################################################
# Build options
DIR		:= src

######################################################################
# Build
all: UPDATE_BUILD_NUMBER
	mkdir -p ./bin
	mkdir -p ./libs
	@for dir in $(DIR); do		\
	make -C $$dir || exit $?;	\
	done

clean:
	@for dir in $(DIR); do		\
	make -C $$dir clean;		\
	done

install:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/data
	mkdir -p $(INSTALL_DIR)/font
	mkdir -p $(INSTALL_DIR)/output
	mkdir -p $(INSTALL_DIR)/plugins
	install -m 777 ./bin/* $(INSTALL_DIR)/
	install -m 777 ./script/nx_diag.conf $(INSTALL_DIR)
	install -m 777 ./script/*.wav $(INSTALL_DIR)/data
	install -m 777 ./font/* $(INSTALL_DIR)/font
	cp -a ./libs/* $(INSTALL_DIR)/plugins

UPDATE_BUILD_NUMBER :
	@echo $$(($$(cat $(BUILD_NUMBER_FILE).txt) + 1)) > $(BUILD_NUMBER_FILE).txt
	@echo
	@echo "Update Build Number: $(MOD_NAME) version $(MAJOR).$(MINOR).$$(cat $(BUILD_NUMBER_FILE).txt) (built by $$(whoami) at $$(date) )"
	@echo
	@echo "#define	DIAG_MODULE_NAME	$(MOD_NAME)	//	Module Name" > $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_DEVICE	\"$(DEVICE_NAME)\"	//	Device Name" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_MAJOR	$(MAJOR)	//	Major Version Number" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_MINOR	$(MINOR)	//	Minor Version Number" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_BUILD	$$(cat $(BUILD_NUMBER_FILE).txt)	//	Build Number" >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_DATE	__DATE__" //	Build Date >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_TIME	__TIME__" //	Build Time >> $(BUILD_INFO_FILE)
	@echo "#define	$(MOD_BASE)_USER	\"$$(whoami)\""	//	Build User >> $(BUILD_INFO_FILE)

