ENCODER_HOT_SWAP_ENABLE = yes
ENCODER_MAP_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
CAPS_WORD_ENABLE = yes
COMBO_ENABLE = yes
OS_DETECTION_ENABLE = yes

ifeq ($(strip $(ENCODER_HOT_SWAP_ENABLE)), yes)
	OPT_DEFS += -DENCODER_HOT_SWAP_ENABLE
	OPT_DEFS += -DMATRIX_REMAP_ENABLE
	SRC += ../common/matrix_remap.c
endif

SRC += ../common/os_key_mapping.c
SRC += keymaps/default/vim.c