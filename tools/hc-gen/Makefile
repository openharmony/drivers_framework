Q = @
BUILD_DIR := build
TARGET := hc-gen


all: $(TARGET)

$(TARGET):
	$(Q)mkdir -p $(BUILD_DIR)
	$(Q)pushd $(BUILD_DIR); cmake ../; popd
	$(Q)make -C $(BUILD_DIR)

test: $(TARGET)
	$(Q) python test/hcgen_test.py $(BUILD_DIR)/$(TARGET)

update_testcase: $(TARGET)
	$(Q) python test/update_case.py $(BUILD_DIR)/$(TARGET)

clean:
	$(Q) rm -rf $(BUILD_DIR)

.PHONY: all clean test $(TARGET)