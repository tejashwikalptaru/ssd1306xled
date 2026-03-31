PIO := $(HOME)/.platformio/penv/bin/pio
EXAMPLES_DIR := ../examples
EXAMPLE ?= sprite_overlap_fix

AVAILABLE := $(notdir $(wildcard $(EXAMPLES_DIR)/*))

.PHONY: setup build simulate clean list help

help: ## Show available commands
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  make %-12s %s\n", $$1, $$2}'
	@echo ""
	@echo "  Use EXAMPLE=<name> to select an example (default: $(EXAMPLE))"
	@echo "  Available: $(AVAILABLE)"

setup: ## Install PlatformIO Core (if not already installed)
	@if [ -f "$(PIO)" ]; then \
		echo "PlatformIO already installed: $$($(PIO) --version)"; \
	else \
		echo "Installing PlatformIO Core..."; \
		curl -fsSL -o /tmp/get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py && \
		python3 /tmp/get-platformio.py; \
	fi

build: ## Build firmware (EXAMPLE=name)
	@if [ ! -d "$(EXAMPLES_DIR)/$(EXAMPLE)" ]; then \
		echo "Error: example '$(EXAMPLE)' not found."; \
		echo "Available: $(AVAILABLE)"; \
		exit 1; \
	fi
	@rm -rf src
	@ln -s $(EXAMPLES_DIR)/$(EXAMPLE) src
	$(PIO) run --environment attiny85

simulate: build ## Build and open Wokwi simulation (EXAMPLE=name)
	@echo ""
	@echo "Firmware built for '$(EXAMPLE)'. To simulate:"
	@echo "  1. Open the simulation/ folder in VS Code"
	@echo "  2. Press F1 -> 'Wokwi: Start Simulator'"

list: ## List available examples
	@echo "Available examples:"
	@for ex in $(AVAILABLE); do \
		if [ "$$ex" = "$(EXAMPLE)" ]; then \
			echo "  * $$ex (default)"; \
		else \
			echo "    $$ex"; \
		fi; \
	done

clean: ## Remove build artifacts
	@rm -rf src .pio
