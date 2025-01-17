#ifndef PACKED_DATA_CONTAINER_H
#define PACKED_DATA_CONTAINER_H

#include "core/resource.h"

class PackedDataContainer : public Resource {
	GDCLASS(PackedDataContainer, Resource);

	enum {
		TYPE_DICT = 0xFFFFFFFF,
		TYPE_ARRAY = 0xFFFFFFFE,
	};

	struct DictKey {
		uint32_t hash;
		Variant key;
		bool operator<(const DictKey &p_key) const { return hash < p_key.hash; }
	};

	PoolVector<uint8_t> data;
	int datalen;

	uint32_t _pack(const Variant &p_data, Vector<uint8_t> &tmpdata, Map<String, uint32_t> &string_cache);

	Variant _iter_init_ofs(const Array &p_iter, uint32_t p_offset);
	Variant _iter_next_ofs(const Array &p_iter, uint32_t p_offset);
	Variant _iter_get_ofs(const Variant &p_iter, uint32_t p_offset);

	Variant _iter_init(const Array &p_iter);
	Variant _iter_next(const Array &p_iter);
	Variant _iter_get(const Variant &p_iter);

	friend class PackedDataContainerRef;
	Variant _key_at_ofs(uint32_t p_ofs, const Variant &p_key, bool &err) const;
	Variant _get_at_ofs(uint32_t p_ofs, const uint8_t *p_buf, bool &err) const;
	uint32_t _type_at_ofs(uint32_t p_ofs) const;
	int _size(uint32_t p_ofs) const;

protected:
	void _set_data(const PoolVector<uint8_t> &p_data);
	PoolVector<uint8_t> _get_data() const;
	static void _bind_methods();

public:
	virtual Variant getvar(const Variant &p_key, bool *r_valid = nullptr) const;
	Error pack(const Variant &p_data);

	int size() const;

	PackedDataContainer();
};

class PackedDataContainerRef : public Reference {
	GDCLASS(PackedDataContainerRef, Reference);

	friend class PackedDataContainer;
	uint32_t offset;
	Ref<PackedDataContainer> from;

protected:
	static void _bind_methods();

public:
	Variant _iter_init(const Array &p_iter);
	Variant _iter_next(const Array &p_iter);
	Variant _iter_get(const Variant &p_iter);
	bool _is_dictionary() const;

	int size() const;
	virtual Variant getvar(const Variant &p_key, bool *r_valid = nullptr) const;

	PackedDataContainerRef();
};

#endif // PACKED_DATA_CONTAINER_H
