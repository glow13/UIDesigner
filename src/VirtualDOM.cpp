#include "VirtualNode.hpp"

#include <geode.devtools/include/API.hpp>
#include <Geode/utils/coro.hpp>
#include <Geode/utils/file.hpp>

class VirtualRoot : public VirtualNode {
public:
	VirtualRoot() : VirtualNode() {}

	matjson::Value exportJSON() override {
		auto obj = VirtualNode::exportJSON();
		obj["type"] = "Root";
		return obj;
	}
};

/// Virtual DOM Manager

VirtualDOMManager* VirtualDOMManager::get() {
	static VirtualDOMManager instance;
	return &instance;
}

VirtualDOMManager::VirtualDOMManager() {
	devtools::waitForDevTools(+[] {
		devtools::registerNode<VirtualNode>(+[](VirtualNode* self) {
			auto manager = VirtualDOMManager::get();
			bool isRoot = typeinfo_cast<VirtualRoot*>(self);

			if (devtools::button((char const*)u8"\ue91e" " Code")) {
				auto out = self->emitCode();
				if (out.back() == '\n')
				    out.pop_back();
				clipboard::write(out + ";");
			}

			devtools::sameLine();

			if (!isRoot && devtools::button((char const*)u8"\ue965" " Delete")) {
				self->removeFromParent();
				return;
			}

			devtools::sameLine();

			auto options = file::FilePickOptions { .filters = {{
				.description = "JSON File", .files = { "*.json"}
			}}};

			if (devtools::button((char const*)u8"\ue92a" " Import")) {
				$async(=) {
					auto json = co_await file::pick(file::PickMode::OpenFile, options);

					if (!json.isOk() || !*json)
						co_return;
					auto data = file::readJson(**json);
					if (!data.isOk())
						co_return;
					
					auto dat = data.unwrap();
					if (isRoot && dat["type"].asString().unwrapOr("") == "Root") {
						self->importJSON(dat);
					} else {
						self->CCNode::addChild(manager->createFromJSON(dat));
					}
				};
			}

			devtools::sameLine();

			if (devtools::button((char const*)u8"\ue967" " Export")) {
				$async(manager, self, options) {
					auto file = co_await file::pick(file::PickMode::SaveFile, options);

					if (file.isOk() && *file) {
						if (file::writeToJson(**file, self->exportJSON()).isErr()) {
							log::warn("Failed to export to {}", file.unwrap());
						}
					}
				};
			}

			if (!isRoot && devtools::button((char const*)u8"\ue91e" " Clone")) {
				if (auto parent = typeinfo_cast<VirtualNode*>(self->getParent()))
					parent->CCNode::addChild(manager->createFromJSON(self->exportJSON()));
			}

			if (!isRoot)
				devtools::sameLine();

			if (devtools::button((char const*)u8"\ue94f" " Child")) {
				self->CCNode::addChild(manager->m_creators[manager->m_creatorNames[self->m_extraData->m_nodeSelection]]());
			}
			devtools::sameLine();
			devtools::nextItemWidth(120.0f);
			devtools::combo("##customnode", self->m_extraData->m_nodeSelection, manager->m_creatorNames);

			devtools::label(fmt::format("Tether Address: {}", (void*)self->m_tether.data()).c_str());
			self->settings();
		});
	});
}

VirtualNode* VirtualDOMManager::initialize(CCLayer* layer) {
	auto vnode = new VirtualRoot;
	vnode->setAnchorPoint({0.5, 0.5});

	layer->addChildAtPosition(vnode, Anchor::Center);
	layer->addChild(vnode->m_tether);
	return vnode;
}

void VirtualDOMManager::registerType(std::string_view name, VirtualCreator ctor) {
	auto [it, inserted] = m_creators.insert({ name.data(), ctor });
	if (inserted) {
		m_creatorNames.push_back(it->first.c_str());
	}
}

VirtualNode* VirtualDOMManager::createFromJSON(matjson::Value obj) {
	auto type = obj["type"].asString().unwrapOr("Node");
	if (type == "Scale 9 Sprite") type = "Nine Slice"; // geode v5 migration
	if (type == "Scale 9 Button") type = "Nine Button";
	auto it = m_creators.find(type);

	if (it != m_creators.end()) {
		auto node = it->second();
		node->importJSON(obj);
		return node;
	} else {
		auto node = m_creators["Node"]();
		node->importJSON(obj);
		return node;
	}
}
