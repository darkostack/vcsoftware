#ifndef MAIN_HPP
#define MAIN_HPP

class Main
{
public:
    explicit Main(void *instances)
        : instance(instances)
    {
    }

    void setup(void);

    void loop(void);

    void *instance;
};

#endif /* MAIN_HPP */
