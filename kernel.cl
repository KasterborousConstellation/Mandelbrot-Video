typedef struct Virtual_Frame {
    int width; int height; int frame_id;int batch_id;
    double x_min;double x_max;double y_min;double y_max;
}Virtual_Frame;
kernel void calculate_frame(global Virtual_Frame* v_frames,global int* data,int num_frames, int accuracy, int redundancy){
    int id = get_global_id(0);
    if(id>=num_frames*redundancy){
        return;
    }
    int worker_id = id%redundancy;
    Virtual_Frame frame = v_frames[id/redundancy];
    int width = frame.width;
    int height = frame.height;
    int frame_id = frame.batch_id;
    int offset = width*height*frame_id;
    
    double x_min = frame.x_min;
    double x_max = frame.x_max;
    double y_min = frame.y_min;
    double y_max = frame.y_max;
    for(int y =worker_id ; y<height; y+=redundancy){
        for(int x=0; x<width;x++){
            double crp = x_min + (double)x/(double)width *(x_max-x_min);
            double cip = y_max - ((double)y/(double)height) *(y_max-y_min); 
            int n;
            double rp = 0;
            double ip = 0;
            for(n=0;n<accuracy;n++){
                double srp = rp * rp;
                double sri = ip * ip;
                if(srp + sri > 4.0){
                    break;
                }
                double new_rp = srp - sri + crp;
                double new_ip = 2*rp*ip +cip;
                rp = new_rp;
                ip = new_ip;
            }
            data[offset + x + y*width] = n;
        }
    }
}