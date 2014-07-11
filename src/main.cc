#include "main.hh"

using namespace cv;

int network[FRAME_HEIGHT * FRAME_WIDTH] = {};

void dumpNetwork() {
  int val;

  for (unsigned int i = 0; i < FRAME_HEIGHT; ++i) {
    std::cout << "  ";
    for (unsigned int j = 0; j < FRAME_WIDTH; ++j) {
      val = network[i * FRAME_WIDTH + j];
      std::cout << (val > 0 ? "\033[32m" : "\033[31m") << val << "\033[0m\t";
    }
    std::cout << std::endl;
  }
}

bool checkNetwork(uint8_t frame[]) {
  int res = 0;

  for (unsigned int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; ++i)
    res += network[i] * frame[i];

  return res > 0;
}

void updateNetwork(uint8_t frame[], signed char diff) {
  for (unsigned int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; ++i)
    network[i] += diff * frame[i];
}

bool parseImage(Mat image, match_map_t matches, bool training, std::string image_name) {
  Mat     frame;
  Mat     img_save = image.clone();
  Mat     tmp_image = image;
  size_t  _x, _y;
  size_t  width, height;
  float   ratio = 1;
  float   real_ratio = 1;
  bool    match, should_match, res = true;

  for (int i = RESIZES; i >= 0; --i) {
    #ifdef VERBOSE
      std::cout << "    ~ Ratio = " << ratio << std::endl;
    #endif

    if (ratio != 1) {
      tmp_image = tmp_image(Rect(0, 0, round(ratio * image.cols), round(ratio * image.rows)));
      resize(image, tmp_image, tmp_image.size(), 0, 0, CV_INTER_AREA);
      image = tmp_image;
      real_ratio *= ratio;
    }

    width = image.cols - FRAME_WIDTH;
    height = image.rows - FRAME_HEIGHT;

    for (size_t y = 0; y <= height; ++y) {
      for (size_t x = 0; x <= width; ++x) {
        frame = image(Rect(x, y, FRAME_WIDTH, FRAME_HEIGHT));
        should_match = false;
        for (auto& match : matches) {
          _x = 1 + (match.first.first.first * real_ratio);
          _y = 1 + (match.first.first.second * real_ratio);
          if ((match.second == NOT_FOUND_YET || match.second == GOOD_SCALE)
              && _x >= x
              && _y >= y
              && ((match.first.first.first + match.first.second.first) * real_ratio) -1 <= (x + FRAME_WIDTH)
              &&  +((match.first.first.second + match.first.second.second) * real_ratio) -1 <= (y + FRAME_HEIGHT)) {
            imwrite("tmp/should_match/" + image_name + "_frame_" + std::to_string(i) + "-" + std::to_string(10000 + x) + "_" + std::to_string(10000 + y) + ".jpg", frame);
            match.second = GOOD_SCALE;
            should_match = true;
          }
        }

        //if (should_match)
        //  imwrite("tmp/should_match/" + image_name + std::to_string(i) + std::to_string(x) + std::to_string(y) + ".jpg", frame);

        if (should_match != (match = checkNetwork(frame.data))) {
          #ifdef VERBOSE
            std::cout << "      ¤ " << (should_match ? "Didn't match when it should" : "Matched when it shouldn't")
              << " [x:" << (x / real_ratio)
              << ", y:" << (y / real_ratio)
              << ", should_match - match:" << should_match - match
              << "]" << std::endl;
          #endif
          res = false;
          if (training)
            updateNetwork(frame.data, should_match - match);
        }
        /*if (should_match) // Display expected matches
          rectangle(img_save,
              Point(std::floor(x / real_ratio), std::floor(y / real_ratio)),
              Point(std::floor((x + FRAME_WIDTH - 1) / real_ratio), std::floor((y + FRAME_HEIGHT - 1) / real_ratio))
              , 125);*/
        if (match && !training) { // Display actual matches
          rectangle(img_save,
              Point(std::floor(x / real_ratio), std::floor(y / real_ratio)),
              Point(std::floor((x + FRAME_WIDTH) / real_ratio), std::floor((y + FRAME_HEIGHT) / real_ratio))
              , 125, CV_FILLED);
        }
      }
    }

    for (auto& match : matches)
      if (match.second == GOOD_SCALE)
        match.second = BAD_SCALE;

    ratio = 1 - ((float) 1 / i) + std::max((float) FRAME_WIDTH / (i * image.cols), (float) FRAME_HEIGHT / (i * image.rows));
  }

  if (!training)
    imwrite("tmp/results/" + image_name, img_save);

  return res;
}

bool readImage(std::string path, Mat& image) {
  image = imread(path, CV_LOAD_IMAGE_GRAYSCALE);
  return !!image.data;
}


float loop(img_map_t images, bool training) {
  Mat image;
  int nb_images = 0;
  int nb_passed = 0;

  std::cout << "- " << (training ? "Training" : "Testing") << std::endl;

  for (auto& img : images) {
    std::cout << "  * " << img.first;
    if (readImage(img.first, image)) {
      std::cout << std::endl;
      //threshold(image, image, 150, 255, CV_THRESH_BINARY);
      imwrite("tmp/" + img.first, image);
      nb_passed += parseImage(image, img.second, training, img.first);
      ++nb_images;
    }
    else
      std::cout << " : /!\\ Error reading file" << std::endl;
  }

  return (float) nb_passed / nb_images;
}

int main()
{
  float res;

  #define Y(x, y, width, height)  \
    {                             \
      {                           \
        {x, y},                   \
        {width, height}           \
      },                          \
      NOT_FOUND_YET               \
    }
  #define X(img, ...)   \
    {                   \
      img,              \
      {                 \
        __VA_ARGS__     \
      }                 \
    },

  img_map_t train_images {
    # include "../training/info.hh"
  };
  img_map_t test_images {
    # include "../testing/info.hh"
  };
  #undef X
  #undef Y

  std::srand(std::time(0));

  for (unsigned int i = 0; i < TRAININGS; ++i) {
    dumpNetwork();
    loop(train_images, true);
  }
  dumpNetwork();

  res = loop(test_images, false);
  std::cout << "Matched correctly for " << res * 100 << "% of pictures" << std::endl;

  return 0;
}

//namedWindow("Display", WINDOW_AUTOSIZE);
//threshold(image, image, 70, 255, CV_THRESH_BINARY);
//imshow("Display", image);
//waitKey(0);
