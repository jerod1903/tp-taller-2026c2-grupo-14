class Crop {
public:
    explicit Crop(CropType type);
    uint8_t stages() const;       // 6, 6, 7, 6 según el tipo
    bool is_ready() const;        // stage == stages() - 1
    // más adelante: riegos por día, días para crecer, precio base
    CropType get_type() const;
    uint8_t get_stage() const;
private:
    CropType type;
    uint8_t stage = 0;
};