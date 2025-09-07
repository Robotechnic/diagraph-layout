VERSION := $(shell grep '^version' typst.toml | awk -F ' = ' '{print $$2}' | tr -d '"')

wasm :
	cd graphviz_interface; \
	make -j16

compile_database:
	bear --output ./graphviz_interface/compile_commands.json -- make wasm

format:
	cd graphviz_interface; \
	make format

link :
	mkdir -p ~/.cache/typst/packages/preview/diagraph-layout
	rm -rf ~/.cache/typst/packages/preview/diagraph-layout/$(VERSION)
	ln -s "$(CURDIR)" ~/.cache/typst/packages/preview/diagraph-layout/$(VERSION)

module :
	mkdir -p ./diagraph-layout
	mkdir -p ./diagraph-layout/graphviz_interface
	mkdir -p ./diagraph-layout/src
	cp ./graphviz_interface/diagraph.wasm ./diagraph-layout/graphviz_interface/diagraph.wasm
	cp ./graphviz_interface/protocol.typ ./diagraph-layout/graphviz_interface/protocol.typ
	cp ./src/*.typ ./diagraph-layout/src/
	cp ./lib.typ ./diagraph-layout/lib.typ
	cp ./README.md ./diagraph-layout/README.md
	cp ./typst.toml ./diagraph-layout/typst.toml
	cp ./LICENSE ./diagraph-layout/LICENSE

clean : clean-link
	cd graphviz_interface; \
	make clean

clean-link:
	rm -rf ~/.cache/typst/packages/preview/diagraph-layout

wasi-stub:
	git clone -n --depth=1 --filter=tree:0 https://github.com/astrale-sharp/wasm-minimal-protocol.git
	cd wasm-minimal-protocol; \
	git sparse-checkout set --no-cone wasi-stub; \
	git checkout
	cd wasm-minimal-protocol/wasi-stub; \
	cargo install --path . 
	rm -rf wasm-minimal-protocol

manual: wasm
	typst compile --root . ./doc/manual.typ