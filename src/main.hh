#ifndef MAIN_HH_
# define MAIN_HH_

# include <map> // std::map
# include <utility> // std::make_pair, std::pair
# include <string> // std::string
# include <iostream>
# include <algorithm> // std::max, std::min
# include <cstdlib>
# include <ctime>
# include <opencv2/opencv.hpp>

//# define VERBOSE

# define FRAME_WIDTH 30
# define FRAME_HEIGHT 20

# define RESIZES 20
# define TRAININGS 15

# define NOT_FOUND_YET 1
# define GOOD_SCALE 2
# define BAD_SCALE 3

typedef std::pair<unsigned int, unsigned int> int_pair_t;
typedef std::map<std::pair<int_pair_t, int_pair_t>, unsigned char> match_map_t;
typedef std::map<std::string, match_map_t> img_map_t;

using namespace cv;

#endif // MAIN_HH_
