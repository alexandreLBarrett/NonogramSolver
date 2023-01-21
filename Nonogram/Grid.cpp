#include "Grid.h"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iomanip>

bool Grid::validate_mask(const mask& source, const validation_mask& mask) const noexcept
{
	for (int i = 0; i < source.size(); ++i) {
		if (mask[i] == CellState::ZERO && source[i])
			return false;
		if (mask[i] == CellState::ONE && !source[i])
			return false;
	}

	return true;
}

Grid::validation_mask Grid::create_mask_for_row(uint8_t row_index) const noexcept
{
	return grid[row_index];
}

Grid::validation_mask Grid::create_mask_for_column(uint8_t col_index) const noexcept
{
	validation_mask mask(height);
	for (uint32_t i = 0; i < height; ++i) {
		mask[i] = grid[i][col_index];
	}
	return mask;
}

vector<Grid::mask> Grid::generate_mask_permutations(const span<const restrictions::value_type>& subset, size length) const noexcept
{
	if (subset.empty())
		return {};

	auto current_val = subset.front();
	auto rest_span = span(subset.begin() + 1, subset.end());
	auto next_size = accumulate(subset.begin(), subset.end(), 0) + (subset.size() - 1);

	vector<mask> masks{};
	mask m(length, false);
	
	for (int i = 0; i <= length - next_size; ++i) {
		auto sub_offset = (i + current_val + 1);
		auto sub_length = length - sub_offset;
		auto submasks = generate_mask_permutations(rest_span, sub_length);

		fill(m.begin(), m.end(), false);

		for (int j = 0; j < current_val; ++j) {
			m[i + j] = true;
		}

		if (submasks.empty()) {
			masks.push_back(m);
		}
		else {
			for_each(
				submasks.begin(), submasks.end(),
				[&](auto sub) {
					auto c = m;
					copy(sub.begin(), sub.end(), c.begin() + sub_offset);
					masks.push_back(c);
				}
			);
		}
	}

	return masks;
}

Grid::probablities Grid::calculate_possibilities_for_line(
	const restrictions& restrictions, const validation_mask& present_mask, const uint8_t line_size) const noexcept
{
	auto perms = generate_mask_permutations(span{ restrictions.begin(), restrictions.end() }, line_size);
	perms.erase(remove_if(perms.begin(), perms.end(), [&](const auto& val) {
		return !validate_mask(val, present_mask);
	}), perms.end());
	
	probablities probs(line_size);
	float increment = 1.0f / perms.size();

	for (auto perm : perms) {
		transform(perm.begin(), perm.end(), probs.begin(), probs.begin(), [&](auto a, auto b) {
			if (a) return b + increment;
			return b;
		});
	}

	return probs;
}

bool Grid::is_solved() const noexcept
{
	return count_empty_slots() == 0;
}

uint32_t Grid::count_empty_slots() const noexcept
{
	uint32_t total = 0;
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			if (grid[y][x] == CellState::NONE)
				++total;
		}
	}
	return total;
}

wstring Grid::to_string(const restrictions& restriction) const noexcept
{
	wstring s;
	for (auto i = 0; i < restriction.size(); ++i) {
		s += to_wstring(restriction[i]);

		if (i < restriction.size() - 1)
			s += ' ';
	}
	return s;
}

Grid::Grid(const string& filename)
{
	ifstream file(filename);
	file >> width;
	file >> height;

	string temp;
	getline(file, temp);

	grid.resize(height);
	for (uint32_t i = 0; i < height; ++i) {
		grid[i].resize(width, CellState::NONE);
	}

	for (uint32_t i = 0; i < width; ++i) {
		getline(file, temp);
		if (temp == "")
		{
			columns_restrictions.emplace_back();
			continue;
		}

		stringstream ss{ temp };

		vector<uint8_t> column;
		while (!ss.eof()) {
			int n;
			ss >> n;
			column.push_back(n);
		}
		columns_restrictions.push_back(column);
	}

	for (uint32_t i = 0; i < height; ++i) {
		getline(file, temp);
		if (temp == "")
		{
			rows_restrictions.emplace_back();
			continue;
		}

		stringstream ss{ temp };

		vector<uint8_t> row;
		while (!ss.eof()) {
			int n;
			ss >> n;
			row.push_back(n);
		}
		rows_restrictions.push_back(row);
	}
}

void Grid::Solve()
{
	uint32_t empty_slot_count = count_empty_slots();
	uint32_t prev_empty_slot_count = 0;

	while (empty_slot_count != prev_empty_slot_count && empty_slot_count != 0) {
		prev_empty_slot_count = empty_slot_count;
		for (int i = 0; i < columns_restrictions.size(); ++i) {
			auto probs = calculate_possibilities_for_line(columns_restrictions[i], create_mask_for_column(i), height);

			for (int j = 0; j < probs.size(); ++j) {
				if (grid[j][i] == CellState::NONE)
					grid[j][i] = toCellState(probs[j]);
			}
		}

		//wcout << L"After columns: " << endl << *this << endl << endl;

		for (int i = 0; i < rows_restrictions.size(); ++i) {
			auto probs = calculate_possibilities_for_line(rows_restrictions[i], create_mask_for_row(i), width);

			for (int j = 0; j < probs.size(); ++j) {
				if (grid[i][j] == CellState::NONE)
					grid[i][j] = toCellState(probs[j]);
			}
		}

		//wcout << L"After rows: " << endl << *this << endl << endl;

		empty_slot_count = count_empty_slots();
	}
}


wostream& operator<<(wostream& o, const Grid& grid) noexcept
{
	auto is_fifth = [](uint32_t i) { return i % 5 == 0; };
	auto is_last = [](uint32_t i, uint32_t size) { return i == size - 1; };
	auto print_cell_value = [&](Grid::CellState state) {
		switch (state) {
			case Grid::CellState::ONE: o << L'█'; break;
			case Grid::CellState::ZERO: o << L'╳'; break;
			case Grid::CellState::NONE: o << L' '; break;
		}
	};

	wstring header{ L'╔'}
		,  footer{ L'╚' }
		, separator{ L'╟' }
		, fifth_separator{ L'╠' };

	for (uint32_t i = 0; i < grid.width; ++i) {
		header += L'═';
		separator += L'─';
		footer += L'═';
		fifth_separator += L'═';

		if (is_fifth(i + 1)) {
			header += L'╦';
			footer += L'╩';
			separator += L'╫';
			fifth_separator += L'╬';
		}
		else {
			header += L'╤';
			separator += L'┼';
			fifth_separator += L'╪';
			footer += L'╧';
		}
	}
	header[header.size() - 1] = L'╗';
	footer[footer.size() - 1] = L'╝';
	separator[separator.size() - 1] = L'╢';
	fifth_separator[fifth_separator.size() - 1] = L'╣';

	auto format_restrictions = [&](const vector<Grid::restrictions>& rest) {
		vector<wstring> restrictions_str;
		for (auto rests : rest)
			restrictions_str.push_back(grid.to_string(rests));

		auto max_rest = max_element(
			restrictions_str.begin(), 
			restrictions_str.end(), 
			[](auto a, auto b) { return a.size() < b.size(); }
		)->size();

		return make_pair(restrictions_str, max_rest);
	};

	auto [rows, max_rows] = format_restrictions(grid.rows_restrictions);
	auto [columns, max_columns] = format_restrictions(grid.columns_restrictions);

	auto pad_stream = [&](const wstring& ws = L" ") { o << setw(max_rows) << ws; };

	for (size_t i = 0; i < max_columns; ++i) {
		pad_stream();
		o << L" ";
		
		for (size_t j = 0; j < columns.size(); ++j) {
			wstring col = columns[j];
			if (col.size() >= max_columns - i) {
				auto offset = i - (max_columns - col.size());
				o << columns[j][offset];
			}
			else {
				o << L" ";
			}

			o << L" ";
		}
		o << endl;
	}

	pad_stream();
	o << header << endl;
	for (uint32_t y = 0; y < grid.height; ++y) {
		pad_stream(rows[y]);
		o << L'║';

		for (uint32_t x = 0; x < grid.width; ++x) {
			print_cell_value(grid.grid[y][x]);
			o << (is_fifth(x + 1) || is_last(x, grid.width) ? L'║' : L'│');
		}

		o << endl;
		if (!is_last(y, grid.height)) {
			pad_stream();
			o << (is_fifth(y + 1) ? fifth_separator : separator) << endl;
		}
	}

	pad_stream();
	o << footer << endl;

	return o;
}
