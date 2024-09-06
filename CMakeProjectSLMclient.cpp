// CMakeProjectSLMClient.cpp : Defines the entry point for the application.
//


#include "CMakeProjectSLMClient.h"
using namespace std;

int main() {
    //  Prepare zmq context and socket
    zmq::context_t context;
    zmq::socket_t socket(context, zmq::socket_type::req);
    std::cout << "Connecting to hello world server..." << std::endl;
    zmq::message_t reply;
    bool zmq_okay = false;
    // send INIT to server - client has been initialized.
    socket.connect("tcp://192.168.122.1:5555");
    socket.send(zmq::buffer(std::string("INIT")), zmq::send_flags::none);
    socket.recv(reply, zmq::recv_flags::none);
    if (reply.to_string() == "ackINIT") {
        std::cout << "Received ackINIT from server" << std::endl;
        zmq_okay = true;
    }
    else {
        std::cout << "Did not receive ackINIT from server" << std::endl;
        return 2;
    }

    // Construct a Blink_SDK instance with Overdrive capability.
    int board_number;
    unsigned int bits_per_pixel = 12U;
    unsigned int n_boards_found = 0U;
    bool constructed_ok = true;
    bool is_nematic_type = true;
    bool RAM_write_enable = true;
    bool use_GPU_if_available = true;
    size_t max_transient_frames = 10U;
    const char static_regional_lut_file = 0;
 
    Blink_SDK sdk(bits_per_pixel, &n_boards_found,
            &constructed_ok, is_nematic_type, RAM_write_enable,
            use_GPU_if_available, 10U, 0);
    bool slm_okay = constructed_ok;
    if (slm_okay){
        std::cout << "SLM constructed OK" << std::endl;
    }
    else {
        std::cout << "Could not construct SLM" << std::endl;
        return 3;
    }

    if (slm_okay && zmq_okay)
    {
        board_number = 1;
        // Used to be just char* but changed due to warning
        const char* lut_file = "C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\LUT Files\\linear.LUT";
        sdk.Load_LUT_file(board_number, lut_file);

        int height = sdk.Get_image_height(board_number);
        int width = sdk.Get_image_width(board_number);
        std::cout << "SLM Width = " << width << std::endl;
        std::cout << "SLM Height = " << height << std::endl;

        while (true) {
            socket.send(zmq::buffer(std::string("xfer")), zmq::send_flags::none);
            socket.recv(reply, zmq::recv_flags::none);
            auto reply_data = reply.data();
            std::size_t image_len = reply.size();
            // int element =  static_cast<int>(static_cast<uint8_t*>(reply_data)[0]);
            std::vector<uint8_t> image(image_len);
            std::memcpy(image.data(), reply_data, image_len);
            // std::vector<uint8_t> image = static_cast<uint8_t*>(reply_data);

            if (image_len > 0) {
                // std::cout << "Received " << image_len << " bytes: " << element << std::endl;
                std::cout << "Received " << image_len << " bytes: " << static_cast<int>(image[0]) << " end " << static_cast<int>(image[9999]) << std::endl;
            }
            else {
                std::cout << "Received empty mask" << std::endl;
            }
            // Generate test mask
            // unsigned char* imageOne = new unsigned char[width * height];
            // int VortexCharge = 5;
            // Generate_LG(imageOne, width, height, VortexCharge, width / 2.0, height / 2.0, false);
            // Write test mask
            bool ExternalTrigger = false;
            bool OutputPulse = false;
            // sdk.Write_image(board_number, image, width * height, ExternalTrigger, OutputPulse, 5000);
            delete[]image;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        sdk.SLM_power(false);
    }
    return 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}