#include "HistsGen.hpp"

int main(int argc, char **argv) {
  HistsGen *app = new HistsGen(argc, argv);
  app->initialize();
  app->run();
  app->finalize();
  return 0;
}
