#include <vector>

namespace nihil::nstd {
	template<typename T>
	std::vector<T> appendVector(std::vector<T>& appendant, std::vector<T>& append)
	{
		std::vector<T> res(appendant.size() + append.size());
		for (int i = 0; i < appendant.size(); i++)
		{
			res[i] = appendant[i];
		}
		for (int i = appendant.size() - 1; i < appendant.size() + append.size(); i++)
		{
			res[i] = append[i - appendant.size()];
		}

		return res;
	}
}