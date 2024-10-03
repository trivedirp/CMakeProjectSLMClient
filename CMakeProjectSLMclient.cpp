// CMakeProjectSLMClient.cpp : Defines the entry point for the application.
//


#include "CMakeProjectSLMClient.h"
using namespace std;
#define RECV_TIMEOUT 1000
#define SEND_TIMEOUT 1000

static zmq::socket_t s_client_socket(zmq::context_t& context, std::string& reconnect_msg) {
    zmq::socket_t client(context, zmq::socket_type::req);
    zmq::message_t reply;
    try {
        client.connect("tcp://192.168.122.1:5555");
        client.setsockopt(ZMQ_RCVTIMEO, RECV_TIMEOUT);
        client.setsockopt(ZMQ_SNDTIMEO, SEND_TIMEOUT);
        // send INIT to server - client has been initialized.
        client.send(zmq::buffer(std::string("INIT")), zmq::send_flags::none);
        client.recv(reply, zmq::recv_flags::none);
        if (reply.to_string() == "ackINIT") {
            std::cout << "Received: " << reply.to_string() << std::endl;
            reconnect_msg = "ackINIT";
        }
        else {
            std::cout << "Failed to receive ackINIT from server... " << std::endl;
        }
    }
    catch (zmq::error_t &e) {
        std::cout << "Failed to receive ackINIT from server... " << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
    }
    return client;
}

int main() {
    //  Prepare zmq context and socket
    zmq::context_t context;
    bool zmq_okay = false;
    zmq::message_t reply;
    std::cout << "Connecting to server..." << std::endl;
    zmq::socket_t client(context, zmq::socket_type::req);
    try {
        client.connect("tcp://192.168.122.1:5555");
        client.setsockopt(ZMQ_RCVTIMEO, RECV_TIMEOUT);
        client.setsockopt(ZMQ_SNDTIMEO, SEND_TIMEOUT);
        // send INIT to server - client has been initialized.
        client.send(zmq::buffer(std::string("INIT")), zmq::send_flags::none);
        client.recv(reply, zmq::recv_flags::none);
        if (reply.to_string() == "ackINIT") {
            std::cout << "Received ackINIT" << std::endl;
            zmq_okay = true;
        }
        else {
            std::cout << "Failed to receive ackINIT from server... " << std::endl;
        }
    }
    catch (zmq::error_t e) {
        std::cout << "Error: " << e.what() << std::endl;
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
        int num_retries = 5;

        while (true) {
            // zmq::pollitem_t items[] = { { client, 0, ZMQ_POLLIN, 0} };
            // zmq::poll(&items[0], 1, REQUEST_TIMEOUT);

            try {
                client.send(zmq::buffer(std::string("ping")), zmq::send_flags::none);
            }
            catch (zmq::error_t &e) {
                std::cout << "zmq send error: " << e.what() << std::endl;
                std::string reconnect_msg = "";
                    for (int i = 0; i <= num_retries; i++) {
                        std::cout << "Retrying Connection..." << std::endl;
                        client.close();
                        client = s_client_socket(context, reconnect_msg);
                        if (reconnect_msg == "ackINIT") {
                            break;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                        if (i == num_retries) {
                            std::cout << "Abondoning Connection" << std::endl;
                            return -1;
                        }
                    }
            }
            try {
                client.recv(reply, zmq::recv_flags::none);
            }
            catch (zmq::error_t &e) {
                std::cout << "zmq recv error: " << e.what() << std::endl;
                /* for (int i = 0; i <= 3; i++) {
                    std::cout << "Retrying Connection..." << std::endl;
                    // client.close();
                    // client = s_client_socket(context);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    if (i == 3) {
                        std::cout << "Abondoning Connection" << std::endl;
                        return -1;
                    }
                }*/
            }
 
            auto reply_data = reply.data();
            std::size_t image_len = reply.size();
            // int element =  static_cast<int>(static_cast<uint8_t*>(reply_data)[0]);
            // unsigned char* image = new unsigned char[width * height];
            // auto image = std::make_unique<unsigned char[]>(image_len);
 
            if (image_len == width * height) {
                std::vector<uint8_t> image(image_len);
                std::memcpy(image.data(), reply_data, image_len);
                // std::cout << "Received " << image_len << " bytes: " << element << std::endl;
                std::cout << "Received " << image_len << " bytes: " << static_cast<int>(image[0]) << " end " << static_cast<int>(image[9999]) << std::endl;
                // Generate test mask
                // unsigned char* imageOne = new unsigned char[width * height];
                // int VortexCharge = 5;
                // Generate_LG(imageOne, width, height, VortexCharge, width / 2.0, height / 2.0, false);
                // Write test mask
                bool ExternalTrigger = false;
                bool OutputPulse = false;
                bool slm_write_success = sdk.Write_image(board_number, image.data(), width * height, ExternalTrigger, OutputPulse, 5000);
                if (slm_write_success) {
                    std::cout << "SLM write success" << std::endl;
                }
                else {
                    std::cout << "SLM write failed" << std::endl;
                }
                image.clear();
            }
            else if (reply.to_string() == "pong") {
                std::cout << "pong..." << std::endl;
            }
            else {
                std::cout << "Waiting to receive mask..." << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        sdk.SLM_power(false);
    }
    return 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}