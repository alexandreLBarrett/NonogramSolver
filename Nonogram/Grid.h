#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <span>
#include <functional>

using namespace std;

class Grid
{
	enum class CellState {
		ONE, ZERO, NONE
	};

	using inner_grid = vector<vector<CellState>>;
	using restrictions = vector<uint8_t>;
	using probablities = vector<float>;
	using mask = vector<bool>;
	using validation_mask = vector<CellState>;
	using size = uint32_t;

	size width, height;
	inner_grid grid;

	vector<restrictions> rows_restrictions;
	vector<restrictions> columns_restrictions;

	const float DELTA = 0.000001f;

	bool isZero(float x) { return x < DELTA; };
	bool isOne(float x) { return x > 1 - DELTA; };
	CellState toCellState(float f) { 
		if (isZero(f)) return CellState::ZERO;
		if (isOne(f)) return CellState::ONE;
		return CellState::NONE;
	}

	bool validate_mask(const mask& source, const validation_mask& mask) const noexcept;

	vector<mask> generate_mask_permutations(const span<const restrictions::value_type>& subset, size length) const noexcept;
	probablities calculate_possibilities_for_line(const restrictions& restriction, const validation_mask& present_mask, const uint8_t line_size) const noexcept;

	template<class MaskFunc>
	void solve_partial_grid(const vector<restrictions>& restrictions, MaskFunc mask_func, const bool on_column) noexcept;
	
	uint32_t count_empty_slots() const noexcept;
	wstring to_string(const restrictions& restriction) const noexcept;
public:
	Grid(const string& filename);

	void solve();
	bool is_solved() const noexcept;

	friend wostream& operator<<(wostream& o, const Grid& grid) noexcept;
};